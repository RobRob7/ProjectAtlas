#include "chunk_pass_vk.h"

#include "vulkan_main.h"

#include "texture_bindings.h"
#include "ubo_bindings.h"

#include "chunk_draw_list.h"
#include "i_chunk_mesh_gpu.h"

#include "render_inputs.h"
#include "chunk_manager.h"
#include "camera.h"
#include "i_light.h"

#include <memory>
#include <cassert>
#include <cstddef>

using namespace Chunk_Constants;
using namespace Gbuffer_Constants;

//--- PUBLIC ---//
ChunkPassVk::ChunkPassVk(VulkanMain& vk)
	: vk_(vk),
	atlas_(vk),
	opaqueUBOBuffer_(vk),
	waterUBOBuffer_(vk),
	opaqueDescriptorSet_(vk),
	waterDescriptorSet_(vk),
	opaquePipeline_(vk),
	waterPipeline_(vk),
	opaqueGBufferUBOBuffer_(vk),
	opaqueGBufferDescriptorSet_(vk),
	opaqueGBufferPipeline_(vk)
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
	waterShader_ = std::make_unique<ShaderModuleVk>(
		vk_.getDevice(), 
		"water/water.vert.spv",
		"water/water.frag.spv"
	);
	opaqueGBufferShader_ = std::make_unique<ShaderModuleVk>(
		vk_.getDevice(),
		"gbuffer/gbuffer.vert.spv",
		"gbuffer/gbuffer.frag.spv"
	);

	atlas_.loadFromFile("blocks.png", true);

	createOpaqueResources();
	createWaterResources();
	createOpaqueGBufferResources();

	createOpaqueDescriptorSet();
	createWaterDescriptorSet();
	createOpaqueGBufferDescriptorSet();

	createOpaquePipeline();
	createWaterPipeline();
	createOpaqueGBufferPipeline();
} // end of init()

void ChunkPassVk::renderOpaque(
	const RenderInputs& in,
	const RenderContext& ctx,
	const glm::mat4& view,
	const glm::mat4& proj,
	int width, int height)
{
	assert(ctx.backend == Backend::Vulkan);

	vk::CommandBuffer cmd = *static_cast<const vk::CommandBuffer*>(ctx.nativeCmd);

	ChunkDrawList list;
	in.world->buildOpaqueDrawList(view, proj, list);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, opaquePipeline_.getPipeline());

	vk::DescriptorSet set = opaqueDescriptorSet_.getSet();
	
	uint32_t drawIndex = 0;
	for (const auto& item : list.items)
	{
		ChunkOpaqueUBO ubo{};
		ubo.u_chunkOrigin = item.chunkOrigin;
		ubo.u_view = view;
		ubo.u_proj = proj;
		ubo.u_screenSize = glm::vec2(width, height);
		ubo.u_clipPlane = glm::vec4(0.0f);
		ubo.u_ambientStrength = in.world->getAmbientStrength();
		ubo.u_useSSAO = 0;

		if (in.camera)
			ubo.u_viewPos = in.camera->getCameraPosition();

		if (in.light)
		{
			ubo.u_lightPos = in.light->getPosition();
			ubo.u_lightColor = in.light->getColor();
		}

		vk::DeviceSize offset =
			static_cast<vk::DeviceSize>(drawIndex) * opaqueUBOStride_;

		opaqueUBOBuffer_.upload(&ubo, sizeof(ubo), offset);

		uint32_t dynamicOffset = static_cast<uint32_t>(offset);

		cmd.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			opaquePipeline_.getLayout(),
			0,
			1, &set,
			1, &dynamicOffset
		);

		item.gpu->drawOpaque(cmd);
		++drawIndex;
	} // end for
} // end of renderOpaque()

void ChunkPassVk::renderWater(
	const RenderInputs& in,
	const RenderContext& ctx,
	const glm::mat4& view,
	const glm::mat4& proj,
	int width, int height)
{

} // end of renderWater()

void ChunkPassVk::renderOpaqueGBuffer(
	const RenderInputs& in,
	const RenderContext& ctx,
	const glm::mat4& view,
	const glm::mat4& proj,
	int width, int height)
{
	assert(ctx.backend == Backend::Vulkan);

	vk::CommandBuffer cmd = *static_cast<const vk::CommandBuffer*>(ctx.nativeCmd);

	ChunkDrawList list;
	in.world->buildOpaqueDrawList(view, proj, list);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, opaqueGBufferPipeline_.getPipeline());

	vk::DescriptorSet set = opaqueGBufferDescriptorSet_.getSet();

	uint32_t drawIndex = 0;
	for (const auto& item : list.items)
	{
		GbufferUBO ubo{};
		ubo.u_chunkOrigin = item.chunkOrigin;
		ubo.u_view = view;
		ubo.u_proj = proj;

		vk::DeviceSize offset =
			static_cast<vk::DeviceSize>(drawIndex) * opaqueGBufferUBOStride_;

		opaqueGBufferUBOBuffer_.upload(&ubo, sizeof(ubo), offset);

		uint32_t dynamicOffset = static_cast<uint32_t>(offset);

		cmd.bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			opaqueGBufferPipeline_.getLayout(),
			0,
			1, &set,
			1, &dynamicOffset
		);

		item.gpu->drawOpaque(cmd);
		++drawIndex;
	} // end for
} // end of renderOpaqueGBuffer()


//--- PRIVATE ---//
void ChunkPassVk::createOpaqueResources()
{
	const uint32_t minAlign =
		vk_.getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;

	opaqueUBOStride_ = sizeof(ChunkOpaqueUBO);
	if (minAlign > 0)
	{
		opaqueUBOStride_ =
			(opaqueUBOStride_ + minAlign - 1) & ~(minAlign - 1);
	}

	constexpr uint32_t MAX_VISIBLE_CHUNKS = 4096;

	opaqueUBOBuffer_.create(
		static_cast<vk::DeviceSize>(opaqueUBOStride_) * MAX_VISIBLE_CHUNKS,
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
} // end of createOpaqueResources()

void ChunkPassVk::createWaterResources()
{
	//waterUBOBuffer_.create(
	//	sizeof(ChunkWaterUBO),
	//	vk::BufferUsageFlagBits::eUniformBuffer,
	//	vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	//);
} // end of createWaterResources()

void ChunkPassVk::createOpaqueGBufferResources()
{
	const uint32_t minAlign =
		vk_.getPhysicalDeviceProperties().limits.minUniformBufferOffsetAlignment;

	opaqueGBufferUBOStride_ = sizeof(GbufferUBO);
	if (minAlign > 0)
	{
		opaqueGBufferUBOStride_ =
			(opaqueGBufferUBOStride_ + minAlign - 1) & ~(minAlign - 1);
	}

	constexpr uint32_t MAX_VISIBLE_CHUNKS = 4096;

	opaqueGBufferUBOBuffer_.create(
		static_cast<vk::DeviceSize>(opaqueGBufferUBOStride_) * MAX_VISIBLE_CHUNKS,
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
} // end of createOpaqueGBufferResources()

void ChunkPassVk::createOpaqueDescriptorSet()
{
	vk::DescriptorSetLayoutBinding uboBinding{};
	uboBinding.binding = TO_API_FORM(UBOBinding::Chunk);
	uboBinding.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
	uboBinding.descriptorCount = 1;
	uboBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

	vk::DescriptorSetLayoutBinding atlasBinding{};
	atlasBinding.binding = TO_API_FORM(TextureBinding::AtlasTex);
	atlasBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	atlasBinding.descriptorCount = 1;
	atlasBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	vk::DescriptorSetLayoutBinding ssaoBinding{};
	ssaoBinding.binding = TO_API_FORM(TextureBinding::SSAORaw);
	ssaoBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	ssaoBinding.descriptorCount = 1;
	ssaoBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

	opaqueDescriptorSet_.createLayout({ uboBinding, atlasBinding, ssaoBinding });

	vk::DescriptorPoolSize uboPool{};
	uboPool.type = vk::DescriptorType::eUniformBufferDynamic;
	uboPool.descriptorCount = 1;

	vk::DescriptorPoolSize atlasPool{};
	atlasPool.type = vk::DescriptorType::eCombinedImageSampler;
	atlasPool.descriptorCount = 1;

	vk::DescriptorPoolSize ssaoPool{};
	ssaoPool.type = vk::DescriptorType::eCombinedImageSampler;
	ssaoPool.descriptorCount = 1;

	opaqueDescriptorSet_.createPool({ uboPool, atlasPool, ssaoPool }, 1);
	opaqueDescriptorSet_.allocate();

	opaqueDescriptorSet_.writeDynamicUniformBuffer(
		TO_API_FORM(UBOBinding::Chunk),
		opaqueUBOBuffer_.getBuffer(),
		sizeof(ChunkOpaqueUBO)
	);

	opaqueDescriptorSet_.writeCombinedImageSampler(
		TO_API_FORM(TextureBinding::AtlasTex),
		atlas_.view(),
		atlas_.sampler()
	);

	opaqueDescriptorSet_.writeCombinedImageSampler(
		TO_API_FORM(TextureBinding::SSAORaw),
		atlas_.view(),
		atlas_.sampler()
	);
} // end of createOpaqueDescriptorSet()

void ChunkPassVk::createWaterDescriptorSet()
{
	//vk::DescriptorSetLayoutBinding uboBinding{};
	//uboBinding.binding = TO_API_FORM(UBOBinding::WaterPass);
	//uboBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	//uboBinding.descriptorCount = 1;
	//uboBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

	//waterDescriptorSet_.createLayout({ uboBinding });

	//vk::DescriptorPoolSize uboPool{};
	//uboPool.type = vk::DescriptorType::eUniformBuffer;
	//uboPool.descriptorCount = 1;

	//waterDescriptorSet_.createPool({ uboPool }, 1);
	//waterDescriptorSet_.allocate();

	//waterDescriptorSet_.writeUniformBuffer(
	//	TO_API_FORM(UBOBinding::WaterPass),
	//	waterUBOBuffer_.getBuffer(),
	//	sizeof(ChunkWaterUBO)
	//);
} // end of createWaterDescriptorSet()

void ChunkPassVk::createOpaqueGBufferDescriptorSet()
{
	vk::DescriptorSetLayoutBinding uboBinding{};
	uboBinding.binding = TO_API_FORM(UBOBinding::Gbuffer);
	uboBinding.descriptorType = vk::DescriptorType::eUniformBufferDynamic;
	uboBinding.descriptorCount = 1;
	uboBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

	opaqueGBufferDescriptorSet_.createLayout({ uboBinding });

	vk::DescriptorPoolSize uboPool{};
	uboPool.type = vk::DescriptorType::eUniformBufferDynamic;
	uboPool.descriptorCount = 1;

	opaqueGBufferDescriptorSet_.createPool({ uboPool });
	opaqueGBufferDescriptorSet_.allocate();

	opaqueGBufferDescriptorSet_.writeDynamicUniformBuffer(
		TO_API_FORM(UBOBinding::Gbuffer),
		opaqueGBufferUBOBuffer_.getBuffer(),
		sizeof(GbufferUBO)
	);
} // end of createOpaqueGBufferDescriptorSet()

void ChunkPassVk::createOpaquePipeline()
{
	GraphicsPipelineDescVk desc{};
	desc.vertShader = opaqueShader_->vertShader();
	desc.fragShader = opaqueShader_->fragShader();

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
} // end of createOpaquePipeline()

void ChunkPassVk::createWaterPipeline()
{
	//GraphicsPipelineDescVk desc{};
	//desc.vertShader = waterShader_->vertShader();
	//desc.fragShader = waterShader_->fragShader();

	//desc.setLayouts = { waterDescriptorSet_.getLayout() };

	//desc.colorFormat = vk_.getSwapChainImageFormat();
	//desc.depthFormat = vk_.getDepthFormat();

	//desc.cullMode = vk::CullModeFlagBits::eBack;
	//desc.frontFace = vk::FrontFace::eClockwise;
	//desc.depthTestEnable = true;
	//desc.depthWriteEnable = false;
	//desc.depthCompareOp = vk::CompareOp::eLessOrEqual;

	//waterPipeline_.create(desc);
} // end of createWaterPipeline()

void ChunkPassVk::createOpaqueGBufferPipeline()
{
	GraphicsPipelineDescVk desc{};
	desc.vertShader = opaqueGBufferShader_->vertShader();
	desc.fragShader = opaqueGBufferShader_->fragShader();

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

