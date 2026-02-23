#ifndef CHUNK_OPAQUE_PASS_VK_H
#define CHUNK_OPAQUE_PASS_VK_H

#include "vulkan_main.h"
#include "chunk_draw_list.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include <vector>
#include <cstdint>

struct RenderInputs;
struct RenderSettings;
struct DrawContext;

class ChunkOpaquePassVk
{
public:
	explicit ChunkOpaquePassVk(VulkanMain& vk);
	~ChunkOpaquePassVk();

	void init(VkFormat swapchainFormat);
	void onSwapchainRecreated(VkFormat swapchainFormat);

	void renderOpaque(const RenderInputs& in, VkFrameContext& frame, const glm::mat4& view, const glm::mat4& proj);
	void renderWater(const RenderInputs& in, VkFrameContext& frame, const glm::mat4& view, const glm::mat4& proj);
	
private:
	VulkanMain& vk_;

	// ---- descriptors ----
	VkDescriptorSetLayout setLayout_{ VK_NULL_HANDLE };
	VkDescriptorPool      descPool_{ VK_NULL_HANDLE };

	std::vector<VkBuffer>       ubo_;
	std::vector<VkDeviceMemory> uboMem_;
	std::vector<VkDescriptorSet> descSet_;

	// ---- atlas (wire later if you want) ----
	VkImage        atlasImage_{ VK_NULL_HANDLE };
	VkDeviceMemory atlasMem_{ VK_NULL_HANDLE };
	VkImageView    atlasView_{ VK_NULL_HANDLE };
	VkSampler      atlasSampler_{ VK_NULL_HANDLE };

	// ---- pipeline ----
	VkPipelineLayout pipeLayout_{ VK_NULL_HANDLE };
	VkPipeline       opaquePipe_{ VK_NULL_HANDLE };
private:
	void destroy();

	void createDescriptorSetLayout();
	void createDescriptorPoolAndSets();
	void createPerFrameUBO();
	void updateDescriptorSets();

	void createPipeline(VkFormat swapchainFormat);
	VkShaderModule createShaderModule(const std::vector<uint8_t>& bytes);

	// dynamic rendering helpers
	void beginRendering(VkCommandBuffer cmd, VkImageView targetView, VkExtent2D extent);
	void endRendering(VkCommandBuffer cmd);

	// swapchain layout transitions (you track layouts already)
	void transitionSwapchain(VkCommandBuffer cmd, uint32_t imageIndex, VkImageLayout newLayout);
};

#endif
