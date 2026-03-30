#include "shadow_map_pass_vk.h"

#include "bindings.h"
#include "constants.h"
#include "render_inputs.h"
#include "chunk_manager.h"

#include "frame_context_vk.h"
#include "utils_vk.h"
#include "vulkan_main.h"
#include "shader_vk.h"

#include "light_vk.h"
#include "chunk_pass_vk.h"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <cstddef>
#include <algorithm>

//--- PUBLIC ---//
ShadowMapPassVk::ShadowMapPassVk(VulkanMain& vk)
	: vk_(vk),
	depthImage_(vk),
	uboBuffer_(vk),
	descriptorSet_(vk),
	pipeline_(vk)
{
} // end of constructor

ShadowMapPassVk::~ShadowMapPassVk() = default;

void ShadowMapPassVk::init()
{
	shader_ = std::make_unique<ShaderModuleVk>(
		vk_.getDevice(),
		"shadowmappass/shadowmappass.vert.spv",
		"shadowmappass/shadowmappass.frag.spv"
	);

	createAttachments();
	createResources();
	createDescriptorSet();
	createPipeline();
} // end of init()

void ShadowMapPassVk::renderOffscreen(
	ChunkPassVk& chunk,
	const RenderInputs& in,
	const FrameContext& frame
)
{
	vk::CommandBuffer cmd = frame.cmd;

	VkUtils::TransitionImageLayout(
		cmd,
		depthImage_.image(),
		vk::ImageAspectFlagBits::eDepth,
		depthLayout_,
		vk::ImageLayout::eDepthAttachmentOptimal,
		1,
		1
	);

	vk::RenderingAttachmentInfo depthAttachment{};
	depthAttachment.imageView = depthImage_.view();
	depthAttachment.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal;
	depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	depthAttachment.clearValue.depthStencil = vk::ClearDepthStencilValue{ 1.0f, 0 };

	vk::RenderingInfo renderingInfo{};
	renderingInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
	renderingInfo.renderArea.extent = vk::Extent2D{
		static_cast<uint32_t>(width_),
		static_cast<uint32_t>(height_)
	};
	renderingInfo.layerCount = 1;
	renderingInfo.pDepthAttachment = &depthAttachment;

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

		// configure light space transform
		glm::vec3 minWS, maxWS;
		if (!in.world->buildVisibleChunkBounds(minWS, maxWS))
		{
			cmd.endRendering();

			VkUtils::TransitionImageLayout(
				cmd,
				depthImage_.image(),
				vk::ImageAspectFlagBits::eDepth,
				depthLayout_,
				vk::ImageLayout::eShaderReadOnlyOptimal,
				1,
				1
			);

			return;
		}
		buildLightSpaceBounds(in, minWS, maxWS);

		uboData_.u_lightSpaceMatrix = lightSpaceMatrix_;
		uboBuffer_.upload(&uboData_, sizeof(uboData_));

		chunk.renderOpaqueShadowMap(
			in,
			frame,
			pipeline_.getLayout(),
			lightView_,
			lightProj_
		);
	}
	cmd.endRendering();

	VkUtils::TransitionImageLayout(
		cmd,
		depthImage_.image(),
		vk::ImageAspectFlagBits::eDepth,
		depthLayout_,
		vk::ImageLayout::eShaderReadOnlyOptimal,
		1,
		1
	);
} // end of render()


//--- PRIVATE ---//
void ShadowMapPassVk::buildLightSpaceBounds(
	const RenderInputs& in,
	const glm::vec3& minWS,
	const glm::vec3& maxWS
)
{
	glm::vec3 centerWS = 0.5f * (minWS + maxWS);

	glm::vec3 lightDir = in.light->getDirection();

	float lightDistance = 200.0f;
	glm::vec3 lightPos = centerWS - lightDir * lightDistance;

	// build light view
	lightView_ = glm::lookAt(
		lightPos,
		centerWS,
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	// build the 8 corners of the visible world-space bounds
	glm::vec3 corners[8] =
	{
		{minWS.x, minWS.y, minWS.z},
		{maxWS.x, minWS.y, minWS.z},
		{minWS.x, maxWS.y, minWS.z},
		{maxWS.x, maxWS.y, minWS.z},
		{minWS.x, minWS.y, maxWS.z},
		{maxWS.x, minWS.y, maxWS.z},
		{minWS.x, maxWS.y, maxWS.z},
		{maxWS.x, maxWS.y, maxWS.z}
	};

	// transform bounds into light space and fit min/max
	glm::vec3 minLS(FLT_MAX);
	glm::vec3 maxLS(-FLT_MAX);

	for (const glm::vec3& c : corners)
	{
		glm::vec4 ls = lightView_ * glm::vec4(c, 1.0f);
		glm::vec3 p(ls);

		minLS = glm::min(minLS, p);
		maxLS = glm::max(maxLS, p);
	} // end for

	// stable shadow mapping
	// padding
	const float xyPad = 8.0f;
	const float zPad = 16.0f;

	float widthLS = maxLS.x - minLS.x;
	float heightLS = maxLS.y - minLS.y;

	float extent = std::max(widthLS, heightLS);
	extent += xyPad * 2.0f;
	extent = std::ceil(extent / CHUNK_SIZE) * CHUNK_SIZE;

	glm::vec3 centerLS = 0.5f * (minLS + maxLS);

	float texelSize = extent / static_cast<float>(std::max(1, width_));

	centerLS.x = std::round(centerLS.x / texelSize) * texelSize;
	centerLS.y = std::round(centerLS.y / texelSize) * texelSize;

	// rebuild snapped X/Y bounds
	minLS.x = centerLS.x - extent * 0.5f;
	maxLS.x = centerLS.x + extent * 0.5f;
	minLS.y = centerLS.y - extent * 0.5f;
	maxLS.y = centerLS.y + extent * 0.5f;

	float nearPlane = -maxLS.z - zPad;
	float farPlane = -minLS.z + zPad;

	// near/far plane clamp
	nearPlane = std::max(0.1f, nearPlane);
	farPlane = std::max(nearPlane + 1.0f, farPlane);

	// build fitted ortho projection
	lightProj_ = glm::orthoRH_ZO(
		minLS.x, maxLS.x,
		minLS.y, maxLS.y,
		nearPlane, farPlane
	);
	lightProj_[1][1] *= -1.0f;

	lightSpaceMatrix_ = lightProj_ * lightView_;
} // end of buildLightSpaceBounds()

void ShadowMapPassVk::createAttachments()
{
	depthImage_.createImage(
		width_,
		height_,
		1,
		false,
		vk::SampleCountFlagBits::e1,
		depthFormat_,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);
	depthImage_.createImageView(
		depthFormat_,
		vk::ImageAspectFlagBits::eDepth,
		vk::ImageViewType::e2D,
		1
	);
	depthImage_.createSampler(
		vk::Filter::eNearest,
		vk::Filter::eNearest,
		vk::SamplerMipmapMode::eNearest,
		vk::SamplerAddressMode::eClampToBorder,
		false
	);

	// RESET
	depthLayout_ = vk::ImageLayout::eUndefined;
} // end of createAttachments()

void ShadowMapPassVk::createResources()
{
	uboBuffer_.create(
		sizeof(ShadowMapPassUBO),
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
} // end of createResources()

void ShadowMapPassVk::createDescriptorSet()
{
	vk::DescriptorSetLayoutBinding uboBinding{};
	uboBinding.binding = TO_API_FORM(ShadowMapPassBinding::UBO);
	uboBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uboBinding.descriptorCount = 1;
	uboBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;

	descriptorSet_.createLayout({ uboBinding});

	vk::DescriptorPoolSize uboPool{};
	uboPool.type = vk::DescriptorType::eUniformBuffer;
	uboPool.descriptorCount = 1;

	descriptorSet_.createPool({ uboPool}, 1);
	descriptorSet_.allocate();

	descriptorSet_.writeUniformBuffer(
		TO_API_FORM(ShadowMapPassBinding::UBO),
		uboBuffer_.getBuffer(),
		sizeof(ShadowMapPassUBO)
	);
} // end of createDescriptorSet()

void ShadowMapPassVk::createPipeline()
{
	GraphicsPipelineDescVk desc{};
	desc.vertShader = shader_->vertShader();
	desc.fragShader = shader_->fragShader();

	vk::PushConstantRange pushRange{};
	pushRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
	pushRange.offset = 0;
	pushRange.size = sizeof(Chunk_Constants::ChunkPushConstants);
	desc.pushConstantRanges = { pushRange };

	vk::VertexInputBindingDescription binding{};
	binding.binding = 0;
	binding.stride = sizeof(Vertex);
	binding.inputRate = vk::VertexInputRate::eVertex;

	vk::VertexInputAttributeDescription attr{};
	attr.location = 0;
	attr.binding = 0;
	attr.format = vk::Format::eR32Uint;
	attr.offset = offsetof(Vertex, sample);

	desc.vertexBinding = binding;
	desc.vertexAttributes = { attr };

	desc.setLayouts = { descriptorSet_.getLayout() };

	desc.depthFormat = depthFormat_;
	desc.depthTestEnable = true;
	desc.depthWriteEnable = true;
	desc.depthCompareOp = vk::CompareOp::eLess;

	desc.cullMode = vk::CullModeFlagBits::eFront;
	desc.frontFace = vk::FrontFace::eClockwise;

	pipeline_.create(desc);
} // end of createPipeline()