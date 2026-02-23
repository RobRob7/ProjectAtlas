#include "renderer_vk.h"

#include "chunk_opaque_pass_vk.h"

#include "chunk_manager.h"
#include "camera.h"

#include "vulkan_main.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <stdexcept>

//--- HELPER ---//
static void transitionSwapchainImage(
	VkCommandBuffer cmd, 
	VkImage image,
	VkImageLayout oldLayout, 
	VkImageLayout newLayout)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage = 0;
	VkPipelineStageFlags destinationStage = 0;

	// ---- old -> COLOR_ATTACHMENT_OPTIMAL ----
	if ((oldLayout == VK_IMAGE_LAYOUT_UNDEFINED || oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) &&
		newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	// ---- COLOR_ATTACHMENT_OPTIMAL -> PRESENT ----
	else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = 0;

		sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}
	else
	{
		// Make it obvious if some other path happens
		throw std::runtime_error("Unsupported swapchain layout transition");
	}


	vkCmdPipelineBarrier(
		cmd,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
} // end of transitionSwapchainImage()

//--- PUBLIC ---//
RendererVk::RendererVk(VulkanMain& vk)
	: vk_(vk)
{
} // end of constructor

RendererVk::~RendererVk()
{

} // end of destructor

void RendererVk::init()
{
	if (!renderSettings_) renderSettings_ = std::make_unique<RenderSettings>();
	if (!chunkOpaque_) chunkOpaque_ = std::make_unique<ChunkOpaquePassVk>(vk_);

	chunkOpaque_->init(vk_.swapChainImageFormat());
} // end of init()

void RendererVk::resize(int w, int h)
{
	width_ = std::max(0, w);
	height_ = std::max(0, h);
} // end of resize()

void RendererVk::renderFrame(const RenderInputs& in)
{
	in.world->update(in.camera->getCameraPosition());

	const glm::mat4 view = in.camera->getViewMatrix();
	const float aspect = (height_ > 0)
		? (static_cast<float>(width_) / static_cast<float>(height_))
		: 1.0f;
	const glm::mat4 proj = in.camera->getProjectionMatrix(aspect);

	VkFrameContext frame{};
	if (!vk_.beginFrame(frame))
	{
		return;
	}

	if (frame.extent.width != width_ || frame.extent.height != height_)
	{
		resize(frame.extent.width, frame.extent.height);
	}

	//VkCommandBuffer cmd = frame.cmd;

	//VkImageLayout old = vk_.swapchainLayout(frame.imageIndex);

	//transitionSwapchainImage(cmd, frame.swapchainImage, old,
	//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	//VkClearValue clear{};
	//clear.color.float32[0] = 0.1f;
	//clear.color.float32[1] = 0.2f;
	//clear.color.float32[2] = 0.4f;
	//clear.color.float32[3] = 1.0f;

	//VkRenderingAttachmentInfo colorAttach{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
	//colorAttach.imageView = frame.swapchainImageView;
	//colorAttach.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	//colorAttach.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//colorAttach.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//colorAttach.clearValue = clear;

	//VkRenderingInfo renderingInfo{ VK_STRUCTURE_TYPE_RENDERING_INFO };
	//renderingInfo.renderArea.offset = { 0, 0 };
	//renderingInfo.renderArea.extent = frame.extent;
	//renderingInfo.layerCount = 1;
	//renderingInfo.colorAttachmentCount = 1;
	//renderingInfo.pColorAttachments = &colorAttach;

	//vkCmdBeginRendering(cmd, &renderingInfo);
	//vkCmdEndRendering(cmd);

	//transitionSwapchainImage(cmd, frame.swapchainImage,
	//	VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	//	VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	//vk_.setSwapchainLayout(frame.imageIndex, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	chunkOpaque_->renderOpaque(in, frame, view, proj);

	vk_.endFrame(frame);
} // end of renderFrame()

RenderSettings& RendererVk::settings()
{
	return *renderSettings_;
} // end of settings()