#ifndef FXAA_PASS_VK_H
#define FXAA_PASS_VK_H

#include "constants.h"

#include "buffer_vk.h"
#include "descriptor_set_vk.h"
#include "graphics_pipeline_vk.h"
#include "image_vk.h"

#include <vulkan/vulkan.hpp>

#include <memory>
#include <vector>

class VulkanMain;
class ShaderModuleVk;
struct FrameContext;

class FXAAPassVk
{
public:
	FXAAPassVk(VulkanMain& vk);
	~FXAAPassVk();

	void init();
	void resize(int w, int h);

	void setInput(ImageVk& input);

	void render(FrameContext& frame);

	ImageVk& getOutputImage() { return outputImage_; }

private:
	void refreshInput(FrameContext& frame);
	void createAttachment();
	void createResources();
	void createDescriptorSets();
	void createPipeline();
private:
	int width_{};
	int height_{};

	VulkanMain& vk_;
	ImageVk* inputImage_{ nullptr };

	ImageVk outputImage_;

	FXAA_Constants::FXAAPassUBO ubo_;

	std::unique_ptr<ShaderModuleVk> shader_;

	std::vector<BufferVk> uboBuffers_;
	std::vector<DescriptorSetVk> descriptorSets_;
	GraphicsPipelineVk pipeline_;

	vk::ImageLayout outputLayout_{ vk::ImageLayout::eUndefined };
};

#endif
