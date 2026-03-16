#include "fxaa_pass_vk.h"

#include "constants.h"

#include "bindings.h"
#include "shader_vk.h"
#include "utils_vk.h"
#include "vulkan_main.h"
#include "image_vk.h"

#include <vulkan/vulkan.hpp>

#include <cstdint>

using namespace FXAA_Constants;

//--- PUBLIC ---//
FXAAPassVk::FXAAPassVk(VulkanMain& vk)
	: vk_(vk),
	outputImage_(vk),
	uboBuffer_(vk),
	descriptorSet_(vk),
	pipeline_(vk)
{
} // end of constuctor

FXAAPassVk::~FXAAPassVk() = default;

void FXAAPassVk::init()
{
	width_ = static_cast<int>(vk_.getSwapChainExtent().width);
	height_ = static_cast<int>(vk_.getSwapChainExtent().height);

	shader_ = std::make_unique<ShaderModuleVk>(
		vk_.getDevice(),
		"fxaapass/fxaa.vert.spv",
		"fxaapass/fxaa.frag.spv"
	);

	createAttachment();
	createResources();
	createDescriptorSet();
	createPipeline();
} // end of init()

void FXAAPassVk::resize(int w, int h)
{
	if (w == 0 || h == 0) return;
	if (w == width_ && h == height_) return;

	vk_.waitIdle();

	width_ = w;
	height_ = h;

	createAttachment();
} // end of resize()

void FXAAPassVk::setInput(ImageVk& input)
{
	inputImage_ = &input;
} // end of setInput()

void FXAAPassVk::render(vk::CommandBuffer cmd)
{
	refreshInput();

	if (!inputImage_ || !uboBuffer_.valid() || !descriptorSet_.valid() || !pipeline_.valid())
	{
		return;
	}

	// update UBO
	ubo_.u_inverseScreenSize = glm::vec2(1.0f / static_cast<float>(width_), 1.0f / static_cast<float>(height_));
	ubo_.u_edgeSharpnessQuality = EDGE_SHARP_QUALITY;
	ubo_.u_edgeThresholdMax = EDGE_THRESH_MAX;
	ubo_.u_edgeThresholdMin = EDGE_THRESH_MIN;

	uboBuffer_.upload(&ubo_, sizeof(ubo_));

	VkUtils::TransitionImageLayout(
		cmd,
		outputImage_.image(),
		vk::ImageAspectFlagBits::eColor,
		outputLayout_,
		vk::ImageLayout::eColorAttachmentOptimal,
		1,
		1
	);

	vk::ClearValue clear{};
	clear.color.float32[0] = 0.0f;
	clear.color.float32[1] = 0.0f;
	clear.color.float32[2] = 0.0f;
	clear.color.float32[3] = 1.0f;

	vk::RenderingAttachmentInfo colorAttach{};
	colorAttach.imageView = outputImage_.view();
	colorAttach.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
	colorAttach.loadOp = vk::AttachmentLoadOp::eDontCare;
	colorAttach.storeOp = vk::AttachmentStoreOp::eStore;
	colorAttach.clearValue = clear;

	vk::RenderingInfo renderingInfo{};
	renderingInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
	renderingInfo.renderArea.extent = vk::Extent2D{
		static_cast<uint32_t>(width_),
		static_cast<uint32_t>(height_)
	};
	renderingInfo.layerCount = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttach;
	renderingInfo.pDepthAttachment = nullptr;

	cmd.beginRendering(renderingInfo);
	{
		vk::Viewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(width_);
		viewport.height = static_cast<float>(height_);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		cmd.setViewport(0, 1, &viewport);

		vk::Rect2D scissor{};
		scissor.offset = vk::Offset2D{ 0, 0 };
		scissor.extent = vk::Extent2D{
			static_cast<uint32_t>(width_),
			static_cast<uint32_t>(height_)
		};
		cmd.setScissor(0, 1, &scissor);

		vk::DescriptorSet set = descriptorSet_.getSet();

		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.getPipeline());
		cmd.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			pipeline_.getLayout(),
			0,
			1, &set,
			0, nullptr
		);

		cmd.draw(3, 1, 0, 0);
	}
	cmd.endRendering();

	VkUtils::TransitionImageLayout(
		cmd,
		outputImage_.image(),
		vk::ImageAspectFlagBits::eColor,
		outputLayout_,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		1,
		1
	);
} // end of render()


//--- PRIVATE ---//
void FXAAPassVk::refreshInput()
{
	if (!inputImage_)
		return;

	if (boundInputImage_ == inputImage_ &&
		inputGeneration_ == inputImage_->generation())
	{
		return;
	}

	descriptorSet_.writeCombinedImageSampler(
		TO_API_FORM(FXAAPassBinding::ForwardColorTex),
		inputImage_->view(),
		inputImage_->sampler()
	);

	boundInputImage_ = inputImage_;
	inputGeneration_ = inputImage_->generation();
} // end of refreshInput()

void FXAAPassVk::createAttachment()
{
	outputImage_.createImage(
		width_,
		height_,
		1,
		false,
		vk::SampleCountFlagBits::e1,
		vk::Format::eR32G32B32A32Sfloat,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);

	outputImage_.createImageView(
		vk::Format::eR32G32B32A32Sfloat,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::e2D,
		1
	);

	outputImage_.createSampler(
		vk::Filter::eLinear,
		vk::Filter::eLinear,
		vk::SamplerMipmapMode::eNearest,
		vk::SamplerAddressMode::eClampToEdge,
		vk::False
	);

	// RESET
	outputLayout_ = vk::ImageLayout::eUndefined;
} // end of createAttachment()

void FXAAPassVk::createResources()
{
	uboBuffer_.create(
		sizeof(FXAAPassUBO),
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
} // end of createResources()

void FXAAPassVk::createDescriptorSet()
{
	vk::DescriptorSetLayoutBinding uboBinding{};
	uboBinding.binding = TO_API_FORM(FXAAPassBinding::UBO);
	uboBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uboBinding.descriptorCount = 1;
	uboBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

	vk::DescriptorSetLayoutBinding inputImgBinding{};
	inputImgBinding.binding = TO_API_FORM(FXAAPassBinding::ForwardColorTex);
	inputImgBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	inputImgBinding.descriptorCount = 1;
	inputImgBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	descriptorSet_.createLayout({ uboBinding, inputImgBinding });
	
	vk::DescriptorPoolSize uboPool;
	uboPool.type = vk::DescriptorType::eUniformBuffer;
	uboPool.descriptorCount = 1;

	vk::DescriptorPoolSize inputImgPool;
	inputImgPool.type = vk::DescriptorType::eCombinedImageSampler;
	inputImgPool.descriptorCount = 1;

	descriptorSet_.createPool({ uboPool, inputImgPool });
	descriptorSet_.allocate();

	descriptorSet_.writeUniformBuffer(
		TO_API_FORM(FXAAPassBinding::UBO),
		uboBuffer_.getBuffer(),
		sizeof(FXAAPassUBO)
	);
} // end of createDescriptorSet()

void FXAAPassVk::createPipeline()
{
	GraphicsPipelineDescVk desc{};
	desc.vertShader = shader_->vertShader();
	desc.fragShader = shader_->fragShader();

	desc.setLayouts = { descriptorSet_.getLayout() };

	desc.colorFormat = vk::Format::eR32G32B32A32Sfloat;
	desc.depthFormat = vk::Format::eUndefined;

	desc.cullMode = vk::CullModeFlagBits::eNone;
	desc.frontFace = vk::FrontFace::eClockwise;
	desc.depthTestEnable = vk::False;
	desc.depthWriteEnable = vk::False;

	pipeline_.create(desc);
} // end of createPipeline()