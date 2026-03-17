#ifndef FOG_PASS_VK_H
#define FOG_PASS_VK_H

#include "constants.h"

#include "buffer_vk.h"
#include "descriptor_set_vk.h"
#include "graphics_pipeline_vk.h"
#include "image_vk.h"

#include <vulkan/vulkan.hpp>

#include <memory>
#include <cstdint>

class VulkanMain;
class ShaderModuleVk;
struct RenderSettings;

class FogPassVk
{
public:
	FogPassVk(VulkanMain& vk, RenderSettings& rs);
	~FogPassVk();

	void init();
	void resize(int w, int h);

	void setInput(ImageVk& inputColor, ImageVk& inputDepth);

	void render(
		vk::CommandBuffer cmd,
		float nearPlane,
		float farPlane,
		float ambStr
	);

	ImageVk& getOutputImage() { return outputImage_; }

private:
	void refreshInput();
	void createAttachment();
	void createResources();
	void createDescriptorSet();
	void createPipeline();
private:
	int width_{};
	int height_{};

	VulkanMain& vk_;
	RenderSettings& rs_;

	ImageVk* inputColorImage_{ nullptr };
	ImageVk* boundInputColorImage_{ nullptr };
	ImageVk* inputDepthImage_{ nullptr };
	ImageVk* boundInputDepthImage_{ nullptr };

	uint64_t inputColorGeneration_ = 0;
	uint64_t inputDepthGeneration_ = 0;

	ImageVk outputImage_;

	Fog_Constants::FogPassUBO ubo_;

	std::unique_ptr<ShaderModuleVk> shader_;

	BufferVk uboBuffer_;
	DescriptorSetVk descriptorSet_;
	GraphicsPipelineVk pipeline_;

	vk::ImageLayout outputLayout_{ vk::ImageLayout::eUndefined };
};

#endif
