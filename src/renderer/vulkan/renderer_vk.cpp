#include "renderer_vk.h"

#include "vulkan_main.h"

#include "render_settings.h"
#include "render_inputs.h"

#include "camera.h"
#include "i_light.h"

#include <glm/glm.hpp>

#include <algorithm>
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
} // end of init()

void RendererVk::resize(int w, int h)
{
	width_ = std::max(0, w);
	height_ = std::max(0, h);
} // end of resize()

void RendererVk::renderFrame(const RenderInputs& in)
{
	//in.world->update(in.camera->getCameraPosition());

	const glm::mat4 view = in.camera->getViewMatrix();
	const float aspect = (height_ > 0)
		? (static_cast<float>(width_) / static_cast<float>(height_))
		: 1.0f;
	glm::mat4 proj = in.camera->getProjectionMatrix(aspect);
	proj[1][1] *= -1.0f;

	FrameContext frame{};
	if (!vk_.beginFrame(frame))
	{
		return;
	}

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

	// BEGIN RENDER
	cmd.beginRendering(renderingInfo);
	{
		Light_Constants::RenderContext ctx{};
		ctx.backend = Light_Constants::RenderContext::Backend::Vulkan;
		ctx.nativeCmd = &cmd;

		if (in.light) in.light->render(ctx, view, proj);
	}

	// END RENDER
	cmd.endRendering();

	transitionSwapchainImage(cmd, frame.swapchainImage,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR);

	vk_.setSwapChainLayout(frame.imageIndex, vk::ImageLayout::ePresentSrcKHR);
	vk_.endFrame(frame);
} // end of renderFrame()

RenderSettings& RendererVk::settings()
{
	return *renderSettings_;
} // end of settings()