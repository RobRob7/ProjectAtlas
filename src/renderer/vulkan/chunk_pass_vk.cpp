#include "chunk_pass_vk.h"

#include "frame_context_vk.h"
#include "vulkan_main.h"

#include "bindings.h"

#include "chunk_draw_list.h"
#include "i_chunk_mesh_gpu.h"

#include "render_inputs.h"
#include "chunk_manager.h"
#include "camera.h"
#include "i_light.h"

#include <memory>
#include <cstddef>

using namespace Chunk_Constants;
using namespace Gbuffer_Constants;

//--- PUBLIC ---//
ChunkPassVk::ChunkPassVk(VulkanMain& vk)
	: vk_(vk),
	atlas_(vk),
	opaqueUBOBuffer_(vk),
	opaqueDescriptorSet_(vk),
	opaquePipeline_(vk),
	opaqueGBufferUBOBuffer_(vk),
	opaqueGBufferDescriptorSet_(vk),
	opaqueGBufferPipeline_(vk),
	opaqueOffscreenUBOBufferReflection_(vk),
	opaqueOffscreenUBOBufferRefraction_(vk),
	opaqueOffscreenDescriptorSetReflection_(vk),
	opaqueOffscreenDescriptorSetRefraction_(vk),
	opaquePipelineOffscreen_(vk)
{
} // end of constructor

ChunkPassVk::~ChunkPassVk() = default;

void ChunkPassVk::init()
{
	opaqueShader_ = std::make_unique<ShaderModuleVk>(
		vk_.getDevice(), 
		"chunk/chunk.vert.spv",
		"chunk/chunk.frag.spv"
	);
	opaqueGBufferShader_ = std::make_unique<ShaderModuleVk>(
		vk_.getDevice(),
		"gbuffer/gbuffer.vert.spv",
		"gbuffer/gbuffer.frag.spv"
	);

	atlas_.loadFromFile("blocks.png", true);

	createOpaqueResources();
	createOpaqueOffscreenResources();
	createOpaqueGBufferResources();

	createOpaqueDescriptorSet();
	createOpaqueOffscreenDescriptorSet();
	createOpaqueGBufferDescriptorSet();

	createOpaquePipeline();
	createOpaqueGBufferPipeline();
} // end of init()

void ChunkPassVk::renderOpaque(
	const RenderInputs& in,
	const FrameContext& frame,
	const glm::mat4& view,
	const glm::mat4& proj,
	int width, int height,
	ChunkOpaqueUBO& ubo
)
{
	vk::CommandBuffer cmd = frame.cmd;

	ChunkDrawList list;
	in.world->buildOpaqueDrawList(view, proj, list);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, opaquePipeline_.getPipeline());

	vk::DescriptorSet set = opaqueDescriptorSet_.getSet();

	ubo.u_view = view;
	ubo.u_proj = proj;
	ubo.u_screenSize = glm::vec2(width, height);
	ubo.u_ambientStrength = in.world->getAmbientStrength();

	ubo.u_viewPos = in.camera->getCameraPosition();

	ubo.u_lightPos = in.light->getPosition();
	ubo.u_lightColor = in.light->getColor();

	opaqueUBOBuffer_.upload(&ubo, sizeof(ubo), 0);

	cmd.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		opaquePipeline_.getLayout(),
		0,
		1, &set,
		0, nullptr
	);
	
	for (const auto& item : list.items)
	{
		ChunkPushConstants pc{};
		pc.u_chunkOrigin = glm::vec4(item.chunkOrigin, 0.0f);

		cmd.pushConstants(
			opaquePipeline_.getLayout(),
			vk::ShaderStageFlagBits::eVertex,
			0,
			sizeof(ChunkPushConstants),
			&pc
		);

		item.gpu->drawOpaque(cmd);
	} // end for
} // end of renderOpaque()

void ChunkPassVk::renderOpaqueOffscreen(
	const RenderInputs& in,
	const FrameContext& frame,
	const glm::mat4& view,
	const glm::mat4& proj,
	int width, int height,
	ChunkOpaqueUBO& ubo,
	DescriptorSetVk& descriptorSet,
	BufferVk& uboBuffer
)
{
	vk::CommandBuffer cmd = frame.cmd;

	ChunkDrawList list;
	in.world->buildOpaqueDrawList(view, proj, list);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, opaquePipelineOffscreen_.getPipeline());

	vk::DescriptorSet set = descriptorSet.getSet();

	ubo.u_view = view;
	ubo.u_proj = proj;
	ubo.u_screenSize = glm::vec2(width, height);
	ubo.u_ambientStrength = in.world->getAmbientStrength();

	uboBuffer.upload(&ubo, sizeof(ubo), 0);

	cmd.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		opaquePipelineOffscreen_.getLayout(),
		0,
		1, &set,
		0, nullptr
	);

	for (const auto& item : list.items)
	{
		ChunkPushConstants pc{};
		pc.u_chunkOrigin = glm::vec4(item.chunkOrigin, 0.0f);

		cmd.pushConstants(
			opaquePipelineOffscreen_.getLayout(),
			vk::ShaderStageFlagBits::eVertex,
			0,
			sizeof(ChunkPushConstants),
			&pc
		);

		item.gpu->drawOpaque(cmd);
	} // end for
} // end of renderOpaqueOffscreen()

void ChunkPassVk::renderOpaqueGBuffer(
	const RenderInputs& in,
	const FrameContext& frame,
	const glm::mat4& view,
	const glm::mat4& proj,
	int width, int height
)
{
	vk::CommandBuffer cmd = frame.cmd;

	ChunkDrawList list;
	in.world->buildOpaqueDrawList(view, proj, list);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, opaqueGBufferPipeline_.getPipeline());

	vk::DescriptorSet set = opaqueGBufferDescriptorSet_.getSet();

	GbufferUBO ubo{};
	ubo.u_view = view;
	ubo.u_proj = proj;

	opaqueGBufferUBOBuffer_.upload(&ubo, sizeof(ubo), 0);

	cmd.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		opaqueGBufferPipeline_.getLayout(),
		0,
		1, &set,
		0, nullptr
	);

	for (const auto& item : list.items)
	{
		ChunkPushConstants pc{};
		pc.u_chunkOrigin = glm::vec4(item.chunkOrigin, 0.0f);

		cmd.pushConstants(
			opaqueGBufferPipeline_.getLayout(),
			vk::ShaderStageFlagBits::eVertex,
			0,
			sizeof(ChunkPushConstants),
			&pc
		);

		item.gpu->drawOpaque(cmd);
	} // end for
} // end of renderOpaqueGBuffer()


//--- PRIVATE ---//
void ChunkPassVk::createOpaqueResources()
{
	opaqueUBOBuffer_.create(
		sizeof(ChunkOpaqueUBO),
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
} // end of createOpaqueResources()

void ChunkPassVk::createOpaqueOffscreenResources()
{
	opaqueOffscreenUBOBufferReflection_.create(
		sizeof(ChunkOpaqueUBO),
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	opaqueOffscreenUBOBufferRefraction_.create(
		sizeof(ChunkOpaqueUBO),
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
} // end of createOpaqueOffscreenResources()

void ChunkPassVk::createOpaqueGBufferResources()
{
	opaqueGBufferUBOBuffer_.create(
		sizeof(GbufferUBO),
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
} // end of createOpaqueGBufferResources()

void ChunkPassVk::createOpaqueDescriptorSet()
{
	vk::DescriptorSetLayoutBinding uboBinding{};
	uboBinding.binding = TO_API_FORM(ChunkBinding::UBO);
	uboBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uboBinding.descriptorCount = 1;
	uboBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

	vk::DescriptorSetLayoutBinding atlasBinding{};
	atlasBinding.binding = TO_API_FORM(ChunkBinding::AtlasTex);
	atlasBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	atlasBinding.descriptorCount = 1;
	atlasBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	vk::DescriptorSetLayoutBinding ssaoBinding{};
	ssaoBinding.binding = TO_API_FORM(ChunkBinding::SSAOTex);
	ssaoBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	ssaoBinding.descriptorCount = 1;
	ssaoBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	opaqueDescriptorSet_.createLayout({ uboBinding, atlasBinding, ssaoBinding });

	vk::DescriptorPoolSize uboPool{};
	uboPool.type = vk::DescriptorType::eUniformBuffer;
	uboPool.descriptorCount = 1;

	vk::DescriptorPoolSize atlasPool{};
	atlasPool.type = vk::DescriptorType::eCombinedImageSampler;
	atlasPool.descriptorCount = 1;

	vk::DescriptorPoolSize ssaoPool{};
	ssaoPool.type = vk::DescriptorType::eCombinedImageSampler;
	ssaoPool.descriptorCount = 1;

	opaqueDescriptorSet_.createPool({ uboPool, atlasPool, ssaoPool }, 1);
	opaqueDescriptorSet_.allocate();

	opaqueDescriptorSet_.writeUniformBuffer(
		TO_API_FORM(ChunkBinding::UBO),
		opaqueUBOBuffer_.getBuffer(),
		sizeof(ChunkOpaqueUBO)
	);

	opaqueDescriptorSet_.writeCombinedImageSampler(
		TO_API_FORM(ChunkBinding::AtlasTex),
		atlas_.view(),
		atlas_.sampler()
	);

	opaqueDescriptorSet_.writeCombinedImageSampler(
		TO_API_FORM(ChunkBinding::SSAOTex),
		atlas_.view(),
		atlas_.sampler()
	);
} // end of createOpaqueDescriptorSet()

void ChunkPassVk::createOpaqueOffscreenDescriptorSet()
{
	vk::DescriptorSetLayoutBinding uboBinding{};
	uboBinding.binding = TO_API_FORM(ChunkBinding::UBO);
	uboBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uboBinding.descriptorCount = 1;
	uboBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

	vk::DescriptorSetLayoutBinding atlasBinding{};
	atlasBinding.binding = TO_API_FORM(ChunkBinding::AtlasTex);
	atlasBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	atlasBinding.descriptorCount = 1;
	atlasBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	vk::DescriptorSetLayoutBinding ssaoBinding{};
	ssaoBinding.binding = TO_API_FORM(ChunkBinding::SSAOTex);
	ssaoBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	ssaoBinding.descriptorCount = 1;
	ssaoBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	// Reflection descriptor set
	opaqueOffscreenDescriptorSetReflection_.createLayout({ uboBinding, atlasBinding, ssaoBinding });

	vk::DescriptorPoolSize uboPool{};
	uboPool.type = vk::DescriptorType::eUniformBuffer;
	uboPool.descriptorCount = 1;

	vk::DescriptorPoolSize atlasPool{};
	atlasPool.type = vk::DescriptorType::eCombinedImageSampler;
	atlasPool.descriptorCount = 1;

	vk::DescriptorPoolSize ssaoPool{};
	ssaoPool.type = vk::DescriptorType::eCombinedImageSampler;
	ssaoPool.descriptorCount = 1;

	opaqueOffscreenDescriptorSetReflection_.createPool({ uboPool, atlasPool, ssaoPool }, 1);
	opaqueOffscreenDescriptorSetReflection_.allocate();

	opaqueOffscreenDescriptorSetReflection_.writeUniformBuffer(
		TO_API_FORM(ChunkBinding::UBO),
		opaqueOffscreenUBOBufferReflection_.getBuffer(),
		sizeof(ChunkOpaqueUBO)
	);

	opaqueOffscreenDescriptorSetReflection_.writeCombinedImageSampler(
		TO_API_FORM(ChunkBinding::AtlasTex),
		atlas_.view(),
		atlas_.sampler()
	);

	opaqueOffscreenDescriptorSetReflection_.writeCombinedImageSampler(
		TO_API_FORM(ChunkBinding::SSAOTex),
		atlas_.view(),
		atlas_.sampler()
	);

	// Refraction descriptor set
	opaqueOffscreenDescriptorSetRefraction_.createLayout({ uboBinding, atlasBinding, ssaoBinding });
	opaqueOffscreenDescriptorSetRefraction_.createPool({ uboPool, atlasPool, ssaoPool }, 1);
	opaqueOffscreenDescriptorSetRefraction_.allocate();

	opaqueOffscreenDescriptorSetRefraction_.writeUniformBuffer(
		TO_API_FORM(ChunkBinding::UBO),
		opaqueOffscreenUBOBufferRefraction_.getBuffer(),
		sizeof(ChunkOpaqueUBO)
	);

	opaqueOffscreenDescriptorSetRefraction_.writeCombinedImageSampler(
		TO_API_FORM(ChunkBinding::AtlasTex),
		atlas_.view(),
		atlas_.sampler()
	);

	opaqueOffscreenDescriptorSetRefraction_.writeCombinedImageSampler(
		TO_API_FORM(ChunkBinding::SSAOTex),
		atlas_.view(),
		atlas_.sampler()
	);
} // end of createOpaqueOffscreenDescriptorSet()

void ChunkPassVk::createOpaqueGBufferDescriptorSet()
{
	vk::DescriptorSetLayoutBinding uboBinding{};
	uboBinding.binding = TO_API_FORM(GbufferBinding::UBO);
	uboBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uboBinding.descriptorCount = 1;
	uboBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

	opaqueGBufferDescriptorSet_.createLayout({ uboBinding });

	vk::DescriptorPoolSize uboPool{};
	uboPool.type = vk::DescriptorType::eUniformBuffer;
	uboPool.descriptorCount = 1;

	opaqueGBufferDescriptorSet_.createPool({ uboPool });
	opaqueGBufferDescriptorSet_.allocate();

	opaqueGBufferDescriptorSet_.writeUniformBuffer(
		TO_API_FORM(GbufferBinding::UBO),
		opaqueGBufferUBOBuffer_.getBuffer(),
		sizeof(GbufferUBO)
	);
} // end of createOpaqueGBufferDescriptorSet()

void ChunkPassVk::createOpaquePipeline()
{
	// normal pipeline
	GraphicsPipelineDescVk desc{};
	desc.vertShader = opaqueShader_->vertShader();
	desc.fragShader = opaqueShader_->fragShader();

	vk::PushConstantRange pushRange{};
	pushRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
	pushRange.offset = 0;
	pushRange.size = sizeof(ChunkPushConstants);
	desc.pushConstantRanges = { pushRange };

	desc.setLayouts = { opaqueDescriptorSet_.getLayout() };

	desc.colorFormat = vk_.getSwapChainImageFormat();
	desc.depthFormat = vk_.getDepthFormat();

	desc.cullMode = vk::CullModeFlagBits::eBack;
	desc.frontFace = vk::FrontFace::eClockwise;
	desc.depthTestEnable = true;
	desc.depthWriteEnable = true;
	desc.depthCompareOp = vk::CompareOp::eLessOrEqual;

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

	opaquePipeline_.create(desc);


	// offscreen pipeline
	desc.setLayouts = { opaqueOffscreenDescriptorSetReflection_.getLayout() };
	desc.colorFormat = vk::Format::eR16G16B16A16Sfloat;
	desc.depthFormat = vk::Format::eD32Sfloat;

	opaquePipelineOffscreen_.create(desc);
} // end of createOpaquePipeline()

void ChunkPassVk::createOpaqueGBufferPipeline()
{
	GraphicsPipelineDescVk desc{};
	desc.vertShader = opaqueGBufferShader_->vertShader();
	desc.fragShader = opaqueGBufferShader_->fragShader();

	vk::PushConstantRange pushRange{};
	pushRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
	pushRange.offset = 0;
	pushRange.size = sizeof(ChunkPushConstants);
	desc.pushConstantRanges = { pushRange };

	desc.setLayouts = { opaqueGBufferDescriptorSet_.getLayout() };

	desc.colorFormat = vk::Format::eR16G16B16A16Sfloat;
	desc.depthFormat = vk::Format::eD32Sfloat;

	desc.cullMode = vk::CullModeFlagBits::eBack;
	desc.frontFace = vk::FrontFace::eClockwise;
	desc.depthTestEnable = true;
	desc.depthWriteEnable = true;
	desc.depthCompareOp = vk::CompareOp::eLessOrEqual;

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

	opaqueGBufferPipeline_.create(desc);
} // end of createOpaqueGBufferPipeline()

