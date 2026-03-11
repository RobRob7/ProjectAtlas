#include "gbuffer_pass_vk.h"

#include "constants.h"

#include "chunk_pass_vk.h"

#include "utils_vk.h"
#include "vulkan_main.h"

#include "vulkan/vulkan.hpp"

#include <cassert>

using namespace Gbuffer_Constants;
using namespace World;

//--- PUBLIC ---//
GBufferPassVk::GBufferPassVk(VulkanMain& vk)
	: vk_(vk),
	gNormalImage_(vk),
	gDepthImage_(vk)
{
} // end of constructor

GBufferPassVk::~GBufferPassVk() = default;

void GBufferPassVk::init()
{
	width_ = static_cast<int>(vk_.getSwapChainExtent().width);
	height_ = static_cast<int>(vk_.getSwapChainExtent().height);

	createAttachments();
} // end of init()

void GBufferPassVk::resize(int w, int h)
{
	if (w == 0 || h == 0) return;
	if (w == width_ && h == height_) return;

	vk_.getDevice().waitIdle();

	width_ = w;
	height_ = h;

	createAttachments();
} // end of resize()

void GBufferPassVk::render(
	ChunkPassVk& chunk,
	const RenderInputs& in,
	const RenderContext& ctx,
	const glm::mat4& view,
	const glm::mat4& proj)
{
	assert(ctx.backend == Backend::Vulkan);

	vk::CommandBuffer cmd = *static_cast<const vk::CommandBuffer*>(ctx.nativeCmd);

	VkUtils::TransitionImageLayout(
		cmd,
		gNormalImage_.image(),
		vk::ImageAspectFlagBits::eColor,
		normalLayout_,
		vk::ImageLayout::eColorAttachmentOptimal,
		1,
		1
	);
	normalLayout_ = vk::ImageLayout::eColorAttachmentOptimal;

	VkUtils::TransitionImageLayout(
		cmd,
		gDepthImage_.image(),
		vk::ImageAspectFlagBits::eDepth,
		depthLayout_,
		vk::ImageLayout::eDepthAttachmentOptimal,
		1,
		1
	);
	depthLayout_ = vk::ImageLayout::eDepthAttachmentOptimal;

	vk::ClearValue normalClear{};
	normalClear.color.float32[0] = 0.0f;
	normalClear.color.float32[1] = 0.0f;
	normalClear.color.float32[2] = 0.0f;
	normalClear.color.float32[3] = 1.0f;

	vk::ClearValue depthClear{};
	depthClear.depthStencil = vk::ClearDepthStencilValue{ 1.0f, 0 };

	vk::RenderingAttachmentInfo colorAttachment{};
	colorAttachment.imageView = gNormalImage_.view();
	colorAttachment.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
	colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttachment.clearValue = normalClear;

	vk::RenderingAttachmentInfo depthAttachment{};
	depthAttachment.imageView = gDepthImage_.view();
	depthAttachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	depthAttachment.clearValue = depthClear;

	vk::RenderingInfo renderingInfo{};
	renderingInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
	renderingInfo.renderArea.extent = vk::Extent2D{
		static_cast<uint32_t>(width_),
		static_cast<uint32_t>(height_)
	};
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachment;
	renderingInfo.pDepthAttachment = &depthAttachment;

	cmd.beginRendering(renderingInfo);
	{
		vk::Viewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(width_);
		viewport.height = static_cast<float>(height_);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cmd.setViewport(0, 1, &viewport);

		vk::Rect2D scissor{};
		scissor.offset = vk::Offset2D{ 0, 0 };
		scissor.extent = vk::Extent2D{
			static_cast<uint32_t>(width_),
			static_cast<uint32_t>(height_)
		};
		cmd.setScissor(0, 1, &scissor);

		// render world
		chunk.renderOpaqueGBuffer(in, ctx, view, proj, width_, height_);
	}
	cmd.endRendering();
	
	VkUtils::TransitionImageLayout(
		cmd,
		gNormalImage_.image(),
		vk::ImageAspectFlagBits::eColor,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		1,
		1
	);
	normalLayout_ = vk::ImageLayout::eShaderReadOnlyOptimal;

	VkUtils::TransitionImageLayout(
		cmd,
		gDepthImage_.image(),
		vk::ImageAspectFlagBits::eDepth,
		vk::ImageLayout::eDepthAttachmentOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		1,
		1
	);
	depthLayout_ = vk::ImageLayout::eShaderReadOnlyOptimal;
} // end of render()


//--- PRIVATE ---//
void GBufferPassVk::createAttachments()
{
	// NORMAL
	gNormalImage_.createImage(
		width_,
		height_,
		1,
		false,
		vk::SampleCountFlagBits::e1,
		normalFormat_,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);
	gNormalImage_.createImageView(
		normalFormat_,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::e2D,
		1
	);
	gNormalImage_.createSampler(
		vk::Filter::eNearest,
		vk::Filter::eNearest,
		vk::SamplerAddressMode::eClampToEdge
	);


	// DEPTH
	gDepthImage_.createImage(
		width_,
		height_,
		1,
		false,
		vk::SampleCountFlagBits::e1,
		depthFormat_,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);
	gDepthImage_.createImageView(
		depthFormat_,
		vk::ImageAspectFlagBits::eDepth,
		vk::ImageViewType::e2D,
		1
	);
	gDepthImage_.createSampler(
		vk::Filter::eNearest,
		vk::Filter::eNearest,
		vk::SamplerAddressMode::eClampToEdge
	);

	// RESET
	normalLayout_ = vk::ImageLayout::eUndefined;
	depthLayout_ = vk::ImageLayout::eUndefined;
} // end of createAttachments()
