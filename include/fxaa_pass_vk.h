#ifndef FXAA_PASS_VK_H
#define FXAA_PASS_VK_H

#include "constants.h"

#include "buffer_vk.h"
#include "descriptor_set_vk.h"
#include "graphics_pipeline_vk.h"
#include "image_vk.h"

#include <memory>
#include <cstdint>

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

	void render(vk::CommandBuffer cmd);

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
	ImageVk* inputImage_{ nullptr };
	ImageVk* boundInputImage_{ nullptr };

	uint64_t inputGeneration_ = 0;

	ImageVk outputImage_;

	FXAA_Constants::FXAAPassUBO ubo_;

	std::unique_ptr<ShaderModuleVk> shader_;

	BufferVk uboBuffer_;
	DescriptorSetVk descriptorSet_;
	GraphicsPipelineVk pipeline_;

	vk::ImageLayout outputLayout_{ vk::ImageLayout::eUndefined };
};

#endif
