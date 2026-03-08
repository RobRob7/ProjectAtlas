#include "renderer_vk.h"

#include "vulkan_main.h"

#include "render_settings.h"
#include "render_inputs.h"

#include "camera.h"
#include "i_light.h"
#include "i_cubemap.h"
#include "chunk_manager.h"
#include "ui_vk.h"

#include <glm/glm.hpp>

#include <stdexcept>

//--- HELPER ---//
static void transitionSwapchainImage(
	vk::CommandBuffer cmd, 
	vk::Image image,
	vk::ImageLayout oldLayout, 
	vk::ImageLayout newLayout)
{
	vk::ImageMemoryBarrier barrier{};
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
	barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vk::PipelineStageFlags sourceStage{};
	vk::PipelineStageFlags destinationStage{};

	// ---- old -> COLOR_ATTACHMENT_OPTIMAL ----
	if ((oldLayout == vk::ImageLayout::eUndefined ||
		oldLayout == vk::ImageLayout::ePresentSrcKHR) &&
		newLayout == vk::ImageLayout::eColorAttachmentOptimal)
	{
		barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}
	// ---- COLOR_ATTACHMENT_OPTIMAL -> PRESENT ----
	else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal &&
		newLayout == vk::ImageLayout::ePresentSrcKHR)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

		sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;
	}
	else
	{
		throw std::runtime_error("Unsupported swapchain layout transition");
	}

	cmd.pipelineBarrier(
		sourceStage,
		destinationStage,
		vk::DependencyFlags{},
		{},
		{},
		barrier
	);
} // end of transitionSwapchainImage()


//--- PUBLIC ---//
RendererVk::RendererVk(VulkanMain& vk)
	: vk_(vk)
{
} // end of constructor

RendererVk::~RendererVk() = default;

void RendererVk::init()
{
	if (!renderSettings_) renderSettings_ = std::make_unique<RenderSettings>();

	if (!chunkPass_) chunkPass_ = std::make_unique<ChunkPassVk>(vk_);
	chunkPass_->init();
} // end of init()

void RendererVk::resize(int w, int h)
{
	if (w <= 0 || h <= 0) return;
	if (w == width_ && h == height_) return;

	width_ = w;
	height_ = h;
} // end of resize()

void RendererVk::renderFrame(const RenderInputs& in, const FrameContext& frame, UIVk* ui)
{
	in.world->update(in.camera->getCameraPosition());

	const glm::mat4 view = in.camera->getViewMatrix();
	const float aspect = (height_ > 0)
		? (static_cast<float>(width_) / static_cast<float>(height_))
		: 1.0f;
	glm::mat4 proj = in.camera->getProjectionMatrix(aspect);
	proj[1][1] *= -1.0f;

	if (frame.extent.width != width_ || frame.extent.height != height_)
	{
		resize(frame.extent.width, frame.extent.height);
	}

	vk::CommandBuffer cmd = frame.cmd;

	vk::ImageLayout old = vk_.getSwapChainLayout(frame.imageIndex);
	transitionSwapchainImage(cmd, frame.swapchainImage, old, vk::ImageLayout::eColorAttachmentOptimal);

	vk::ClearValue clear{};
	clear.color.float32[0] = 0.1f;
	clear.color.float32[1] = 0.1f;
	clear.color.float32[2] = 0.1f;
	clear.color.float32[3] = 1.0f;

	vk::RenderingAttachmentInfo colorAttach{};
	colorAttach.imageView = frame.swapchainImageView;
	colorAttach.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
	colorAttach.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttach.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttach.clearValue = clear;

	vk::RenderingAttachmentInfo depthAttach{};
	depthAttach.imageView = frame.depthImageView;
	depthAttach.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
	depthAttach.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttach.storeOp = vk::AttachmentStoreOp::eStore;
	depthAttach.clearValue.depthStencil = vk::ClearDepthStencilValue{ 1.0f, 0 };

	vk::RenderingInfo renderingInfo{};
	renderingInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
	renderingInfo.renderArea.extent = frame.extent;
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttach;
	renderingInfo.pDepthAttachment = &depthAttach;

	// FORWARD RENDER
	cmd.beginRendering(renderingInfo);
	{
		vk::Viewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(frame.extent.width);
		viewport.height = static_cast<float>(frame.extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cmd.setViewport(0, 1, &viewport);

		vk::Rect2D scissor{};
		scissor.offset = vk::Offset2D{ 0, 0 };
		scissor.extent = frame.extent;
		cmd.setScissor(0, 1, &scissor);

		RenderContext ctx{};
		ctx.backend = RenderContext::Backend::Vulkan;
		ctx.nativeCmd = &cmd;

		if (chunkPass_)
		{
			chunkPass_->renderOpaque(in, ctx, view, proj, width_, height_);
			//chunkPass_->renderWater(in, ctx, view, proj, width_, height_);
		}
		if (in.light) in.light->render(ctx, view, proj);
		if (in.skybox) in.skybox->render(ctx, view, proj);

		if (ui)
		{
			ui->render(cmd);
		}
	}
	cmd.endRendering();

	transitionSwapchainImage(cmd, frame.swapchainImage,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR);

	vk_.setSwapChainLayout(frame.imageIndex, vk::ImageLayout::ePresentSrcKHR);
	//vk_.endFrame(frame);
} // end of renderFrame()

RenderSettings& RendererVk::settings()
{
	return *renderSettings_;
} // end of settings()