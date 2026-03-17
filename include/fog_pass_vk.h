#ifndef FOG_PASS_VK_H
#define FOG_PASS_VK_H

#include "constants.h"
#include "frame_context_vk.h"

#include "buffer_vk.h"
#include "descriptor_set_vk.h"
#include "graphics_pipeline_vk.h"
#include "image_vk.h"

#include <vulkan/vulkan.hpp>

#include <memory>
#include <vector>

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
		FrameContext& frame,
		float nearPlane,
		float farPlane,
		float ambStr
	);

	ImageVk& getOutputImage() { return outputImage_; }

private:
	void refreshInput(FrameContext& frame);
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
	ImageVk* inputDepthImage_{ nullptr };

	ImageVk outputImage_;

	Fog_Constants::FogPassUBO ubo_;

	std::unique_ptr<ShaderModuleVk> shader_;

	std::vector<BufferVk> uboBuffers_;
	std::vector<DescriptorSetVk> descriptorSets_;
	GraphicsPipelineVk pipeline_;

	vk::ImageLayout outputLayout_{ vk::ImageLayout::eUndefined };
};

#endif
