#ifndef SSAO_PASS_VK_H
#define SSAO_PASS_VK_H

#include "constants.h"

#include "buffer_vk.h"
#include "descriptor_set_vk.h"
#include "graphics_pipeline_vk.h"
#include "image_vk.h"

#include <vulkan/vulkan.hpp>

#include <memory>
#include <array>

class VulkanMain;
class ShaderModuleVk;
struct FrameContext;

class SSAOPassVk
{
public:
	SSAOPassVk(VulkanMain& vk, ImageVk& gNormalImage, ImageVk& gDepthImage);
	~SSAOPassVk();

	void init();
	void resize(int w, int h);

	void renderOffscreen(
		const FrameContext& frame,
		const glm::mat4& proj
	);

	ImageVk& ssaoBlurImage() { return ssaoBlurImage_; }

private:
	void createAttachments();
	void createBuffers();
	void createDescriptorSets();
	void createPipelines();

	void createNoiseTexture();
	void createKernel();
private:
	VulkanMain& vk_;

	int width_{};
	int height_{};

	SSAO_Constants::SSAORawUBO rawUBO_{};
	SSAO_Constants::SSAOBlurUBO blurUBO_{};

	std::unique_ptr<ShaderModuleVk> ssaoRawShader_;
	std::unique_ptr<ShaderModuleVk> ssaoBlurShader_;

	ImageVk& gNormalImage_;
	ImageVk& gDepthImage_;
	ImageVk ssaoNoiseImage_;

	ImageVk ssaoRawImage_;
	ImageVk ssaoBlurImage_;

	BufferVk ssaoRawUBOBuffer_;
	BufferVk ssaoBlurUBOBuffer_;

	DescriptorSetVk ssaoRawDescriptorSet_;
	DescriptorSetVk ssaoBlurDescriptorSet_;

	GraphicsPipelineVk ssaoRawPipeline_;
	GraphicsPipelineVk ssaoBlurPipeline_;

	vk::Format noiseFormat_ = vk::Format::eR16G16B16A16Sfloat;
	vk::Format singleChannelFormat_ = vk::Format::eR8Unorm;

	vk::ImageLayout singleChannelRawLayout_ = vk::ImageLayout::eUndefined;
	vk::ImageLayout singleChannelBlurLayout_ = vk::ImageLayout::eUndefined;

	std::array<glm::vec4, SSAO_Constants::MAX_KERNEL_SIZE> samples_{};
};

#endif
