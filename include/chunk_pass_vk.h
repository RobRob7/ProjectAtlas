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
		const FrameContext& frame, 
		const glm::mat4& view, 
		const glm::mat4& proj,
		int width, int height,
		Chunk_Constants::ChunkOpaqueUBO& ubo
	);
	void renderOpaqueOffscreen(
		const RenderInputs& in,
		const FrameContext& frame,
		const glm::mat4& view,
		const glm::mat4& proj,
		int width, int height,
		Chunk_Constants::ChunkOpaqueUBO& ubo,
		DescriptorSetVk& descriptorSet,
		BufferVk& uboBuffer
	);
	void renderOpaqueGBuffer(
		const RenderInputs& in,
		const FrameContext& frame,
		const glm::mat4& view,
		const glm::mat4& proj,
		int width, int height
	);

	BufferVk& getOpaqueOffscreenUBOBufferReflection() { return opaqueOffscreenUBOBufferReflection_; }
	BufferVk& getOpaqueOffscreenUBOBufferRefraction() { return opaqueOffscreenUBOBufferRefraction_; }

	DescriptorSetVk& getOpaqueOffscreenDescriptorSetReflection() { return opaqueOffscreenDescriptorSetReflection_; }
	DescriptorSetVk& getOpaqueOffscreenDescriptorSetRefraction() { return opaqueOffscreenDescriptorSetRefraction_; }

private:
	void createOpaqueResources();
	void createOpaqueOffscreenResources();
	void createOpaqueGBufferResources();

	void createOpaqueDescriptorSet();
	void createOpaqueOffscreenDescriptorSet();
	void createOpaqueGBufferDescriptorSet();

	void createOpaquePipeline();
	void createOpaqueGBufferPipeline();
private:
	VulkanMain& vk_;

	std::unique_ptr<ShaderModuleVk> opaqueShader_;
	std::unique_ptr<ShaderModuleVk> opaqueGBufferShader_;

	Texture2DVk atlas_;

	BufferVk opaqueUBOBuffer_;
	BufferVk opaqueOffscreenUBOBufferReflection_;
	BufferVk opaqueOffscreenUBOBufferRefraction_;
	BufferVk opaqueGBufferUBOBuffer_;

	DescriptorSetVk opaqueDescriptorSet_;
	DescriptorSetVk opaqueOffscreenDescriptorSetReflection_;
	DescriptorSetVk opaqueOffscreenDescriptorSetRefraction_;
	DescriptorSetVk opaqueGBufferDescriptorSet_;

	GraphicsPipelineVk opaquePipeline_;
	GraphicsPipelineVk opaquePipelineOffscreen_;
	GraphicsPipelineVk opaqueGBufferPipeline_;

	uint32_t opaqueUBOStride_{ 0 };
	uint32_t opaqueOffscreenUBOStride_{ 0 };
	uint32_t opaqueGBufferUBOStride_{ 0 };
};

#endif
