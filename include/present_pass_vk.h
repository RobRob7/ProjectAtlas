#ifndef PRESENT_PASS_VK_H
#define PRESENT_PASS_VK_H

#include "descriptor_set_vk.h"
#include "graphics_pipeline_vk.h"
#include "image_vk.h"

#include <memory>

class VulkanMain;
class ShaderModuleVk;

class PresentPassVk
{
public:
	PresentPassVk(VulkanMain& vk, ImageVk& inputImage);
	~PresentPassVk();

	void init();
	void refreshInput();

	void render(vk::CommandBuffer cmd);

private:
	void createDescriptorSet();
	void createPipeline();
private:
	VulkanMain& vk_;
	ImageVk& inputImage_;

	std::unique_ptr<ShaderModuleVk> shader_;

	DescriptorSetVk descriptorSet_;
	GraphicsPipelineVk pipeline_;
};

#endif
