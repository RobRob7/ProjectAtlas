#ifndef CHUNK_PASS_VK_H
#define CHUNK_PASS_VK_H

#include "constants.h"

#include "shader_vk.h"
#include "texture_2d_vk.h"
#include "buffer_vk.h"
#include "descriptor_set_vk.h"
#include "graphics_pipeline_vk.h"

#include <glm/glm.hpp>

#include <memory>
#include <cstdint>

class VulkanMain;
struct RenderInputs;
struct RenderSettings;
struct DrawContext;
struct FrameContext;

class ChunkPassVk
{
public:
	explicit ChunkPassVk(VulkanMain& vk);
	~ChunkPassVk();

	void init();

	void renderOpaque(
		const RenderInputs& in, 
		const RenderContext& ctx, 
		const glm::mat4& view, 
		const glm::mat4& proj,
		int width, int height);
	void renderWater(
		const RenderInputs& in, 
		const RenderContext& ctx, 
		const glm::mat4& view, 
		const glm::mat4& proj,
		int width, int height);
	void renderOpaqueGBuffer(
		const RenderInputs& in,
		const RenderContext& ctx,
		const glm::mat4& view,
		const glm::mat4& proj,
		int width, int height);

private:
	void createOpaqueResources();
	void createWaterResources();
	void createOpaqueGBufferResources();

	void createOpaqueDescriptorSet();
	void createWaterDescriptorSet();
	void createOpaqueGBufferDescriptorSet();

	void createOpaquePipeline();
	void createWaterPipeline();
	void createOpaqueGBufferPipeline();
private:
	VulkanMain& vk_;

	std::unique_ptr<ShaderModuleVk> opaqueShader_;
	std::unique_ptr<ShaderModuleVk> waterShader_;
	std::unique_ptr<ShaderModuleVk> opaqueGBufferShader_;

	Texture2DVk atlas_;

	BufferVk opaqueUBOBuffer_;
	BufferVk waterUBOBuffer_;
	BufferVk opaqueGBufferUBOBuffer_;

	DescriptorSetVk opaqueDescriptorSet_;
	DescriptorSetVk waterDescriptorSet_;
	DescriptorSetVk opaqueGBufferDescriptorSet_;

	GraphicsPipelineVk opaquePipeline_;
	GraphicsPipelineVk waterPipeline_;
	GraphicsPipelineVk opaqueGBufferPipeline_;

	uint32_t opaqueUBOStride_{ 0 };
	uint32_t opaqueGBufferUBOStride_{ 0 };
};

#endif
