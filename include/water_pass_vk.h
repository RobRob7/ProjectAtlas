#ifndef WATER_PASS_VK_H
#define WATER_PASS_VK_H

#include "image_vk.h"
#include "texture_2d_vk.h"

#include "buffer_vk.h"
#include "descriptor_set_vk.h"
#include "graphics_pipeline_vk.h"

#include <vulkan/vulkan.hpp>

class VulkanMain;
class ShaderModuleVk;
struct RenderInputs;
struct RenderContext;
class ChunkPassVk;
struct FrameContext;

class WaterPassVk
{
public:
	WaterPassVk(VulkanMain& vk);
	~WaterPassVk();

	void init();
	void resize(int w, int h);

	void renderOffscreen(
		const FrameContext& frame,
		ChunkPassVk& chunk,
		const RenderInputs& in
	);

	void renderWater(
		FrameContext& frame,
		ChunkPassVk& chunk, 
		const RenderInputs& in
	);

	ImageVk& getReflColorImage() { return reflColorImage_; }
	ImageVk& getReflDepthImage() { return reflDepthImage_; }

	ImageVk& getRefrColorImage() { return refrColorImage_; }
	ImageVk& getRefrDepthImage() { return refrDepthImage_; }

private:
	void createAttachments();
	void createResources();
	void createDescriptorSet();
	void createPipeline();
	void waterPass(
		const FrameContext& frame,
		ChunkPassVk& chunk, 
		const RenderInputs& in
	);
	void waterReflectionPass(
		const FrameContext& frame,
		ChunkPassVk& chunk, 
		const RenderInputs& in
	) const;
	void waterRefractionPass(
		const FrameContext& frame,
		ChunkPassVk& chunk, 
		const RenderInputs& in
	) const;
private:
	VulkanMain& vk_;

	int factor_{};

	int width_{ 0 };
	int height_{ 0 };
	int fullW_{ 0 };
	int fullH_{ 0 };

	ImageVk reflColorImage_;
	ImageVk reflDepthImage_;

	vk::ImageLayout reflColorLayout_ = vk::ImageLayout::eUndefined;
	vk::ImageLayout reflDepthLayout_ = vk::ImageLayout::eUndefined;

	ImageVk refrColorImage_;
	ImageVk refrDepthImage_;

	vk::ImageLayout refrColorLayout_ = vk::ImageLayout::eUndefined;
	vk::ImageLayout refrDepthLayout_ = vk::ImageLayout::eUndefined;

	vk::Format colorFormat_ = vk::Format::eR16G16B16A16Sfloat;
	vk::Format depthFormat_ = vk::Format::eD32Sfloat;

	std::unique_ptr<ShaderModuleVk> shader_;

	Texture2DVk dudvTex_;
	Texture2DVk normalTex_;

	BufferVk uboBuffer_;
	DescriptorSetVk descriptorSet_;
	GraphicsPipelineVk pipeline_;
};

#endif
