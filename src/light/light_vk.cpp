#include "light_vk.h"

#include "vulkan_main.h"
#include "shader_vk.h"

#include "ubo_bindings.h"

#include <vulkan/vulkan.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <array>
#include <cassert>

using namespace Light_Constants;

//--- PUBLIC ---//
LightVk::LightVk(VulkanMain& vk, const glm::vec3& pos, const glm::vec3& color)
	: vk_(vk), position_(pos)
{
	setColor(color);

	shader_ = std::make_unique<ShaderModuleVk>(vk_.getDevice(), "light/light.vert.spv", "light/light.frag.spv");
} // end of constructor

LightVk::~LightVk() = default;

void LightVk::init()
{
	createVertexBuffer();
	createPipeline();
	createUBO();
	createDesciptorSet();
} // end of init()

void LightVk::render(const RenderContext& ctx, const glm::mat4& view, const glm::mat4& proj)
{
	assert(ctx.backend == RenderContext::Backend::Vulkan && "Must be Vulkan render context!");

	if (!pipeline_ || !vertexBuffer_) return;

	vk::CommandBuffer cmd = *static_cast<const vk::CommandBuffer*>(ctx.nativeCmd);

	vk::Extent2D extent = vk_.getSwapChainExtent();

	vk::Viewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor{};
	scissor.offset = vk::Offset2D{ 0, 0 };
	scissor.extent = extent;

	cmd.setViewport(0, 1, &viewport);
	cmd.setScissor(0, 1, &scissor);

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.get());

	vk::DeviceSize offset = 0;
	cmd.bindVertexBuffers(0, 1, &vertexBuffer_.get(), &offset);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), position_);

	LightUBO ubo{};
	ubo.model = model;
	ubo.view = view;
	ubo.proj = proj;
	ubo.color = glm::vec4(color_, 1.0f);

	auto device = vk_.getDevice();
	auto rvMap = device.mapMemory(uboMemory_.get(), 0, sizeof(LightUBO));
	if (rvMap.result != vk::Result::eSuccess)
		throw std::runtime_error("mapMemory(UBO) failed: " + vk::to_string(rvMap.result));

	std::memcpy(rvMap.value, &ubo, sizeof(LightUBO));
	device.unmapMemory(uboMemory_.get());

	cmd.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		pipelineLayout_.get(),
		0,                      // firstSet = 0
		1, &descSet_,
		0, nullptr
	);

	cmd.draw(36, 1, 0, 0);
} // end of render()


//--- PRIVATE ---//
void LightVk::createVertexBuffer()
{
	// CUBE_VERTICES: 8 floats per vertex (pos3, normal3, uv2)
	constexpr size_t floatsPerVert = 8;
	const size_t vertCount = CUBE_VERTICES.size() / floatsPerVert;

	std::vector<VertexLight> verts;
	verts.reserve(vertCount);

	for (size_t i = 0; i < vertCount; ++i)
	{
		const size_t base = i * floatsPerVert;
		verts.push_back(VertexLight{
			glm::vec3{
				CUBE_VERTICES[base + 0],
				CUBE_VERTICES[base + 1],
				CUBE_VERTICES[base + 2]
			}
			});
	}

	vertexBufferSize_ = sizeof(VertexLight) * verts.size();

	vk::BufferCreateInfo createInfo{};
	createInfo.size = vertexBufferSize_;
	createInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
	createInfo.sharingMode = vk::SharingMode::eExclusive;

	vk::Device device = vk_.getDevice();

	{
		vk::ResultValue rv = device.createBufferUnique(createInfo);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("createBufferUnique failed: " + vk::to_string(rv.result));
		}
		vertexBuffer_ = std::move(rv.value);
	}

	vk::MemoryRequirements req = device.getBufferMemoryRequirements(vertexBuffer_.get());

	uint32_t memType = vk_.findMemoryType(
		req.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	vk::MemoryAllocateInfo mai{};
	mai.allocationSize = req.size;
	mai.memoryTypeIndex = memType;

	{
		vk::ResultValue rv = device.allocateMemoryUnique(mai);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("allocateMemoryUnique failed: " + vk::to_string(rv.result));
		}
		vertexMemory_ = std::move(rv.value);
	}

	{
		vk::Result res = device.bindBufferMemory(vertexBuffer_.get(), vertexMemory_.get(), 0);
		if (res != vk::Result::eSuccess)
		{
			throw std::runtime_error("bindBufferMemory failed: " + vk::to_string(res));
		}
	}

	{
		vk::ResultValue rv = device.mapMemory(vertexMemory_.get(), 0, vertexBufferSize_);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("mapMemory failed: " + vk::to_string(rv.result));
		}
		std::memcpy(rv.value, verts.data(), static_cast<std::size_t>(vertexBufferSize_));
	}

	device.unmapMemory(vertexMemory_.get());
} // end of createVertexBuffer()

void LightVk::createPipeline()
{
	vk::Device device = vk_.getDevice();

	vk::PipelineShaderStageCreateInfo stages[2]{};
	stages[0].stage = vk::ShaderStageFlagBits::eVertex;
	stages[0].module = shader_->vertShader();
	stages[0].pName = "main";
	stages[1].stage = vk::ShaderStageFlagBits::eFragment;
	stages[1].module = shader_->fragShader();
	stages[1].pName = "main";

	vk::VertexInputBindingDescription binding{};
	binding.binding = 0;
	binding.stride = sizeof(VertexLight);
	binding.inputRate = vk::VertexInputRate::eVertex;

	vk::VertexInputAttributeDescription attr{};
	attr.location = 0;
	attr.binding = 0;
	attr.format = vk::Format::eR32G32B32Sfloat;
	attr.offset = offsetof(VertexLight, pos);

	vk::PipelineVertexInputStateCreateInfo vi{};
	vi.vertexBindingDescriptionCount = 1;
	vi.pVertexBindingDescriptions = &binding;
	vi.vertexAttributeDescriptionCount = 1;
	vi.pVertexAttributeDescriptions = &attr;

	vk::PipelineInputAssemblyStateCreateInfo ia{};
	ia.topology = vk::PrimitiveTopology::eTriangleList;

	vk::PipelineViewportStateCreateInfo vp{};
	vp.viewportCount = 1;
	vp.scissorCount = 1;

	vk::PipelineRasterizationStateCreateInfo rs{};
	rs.polygonMode = vk::PolygonMode::eFill;
	rs.cullMode = vk::CullModeFlagBits::eBack;
	rs.frontFace = vk::FrontFace::eCounterClockwise;
	rs.lineWidth = 1.0f;

	vk::PipelineMultisampleStateCreateInfo ms{};
	ms.rasterizationSamples = vk::SampleCountFlagBits::e1;

	vk::PipelineColorBlendAttachmentState cba{};
	cba.colorWriteMask =
		vk::ColorComponentFlagBits::eR |
		vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eA;
	cba.blendEnable = vk::False;

	vk::PipelineColorBlendStateCreateInfo cb{};
	cb.attachmentCount = 1;
	cb.pAttachments = &cba;

	std::array<vk::DynamicState, 2> dynStates =
	{
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor
	};
	vk::PipelineDynamicStateCreateInfo dyn{};
	dyn.dynamicStateCount = static_cast<uint32_t>(dynStates.size());
	dyn.pDynamicStates = dynStates.data();

	// UBO
	vk::DescriptorSetLayoutBinding uboBinding{};
	uboBinding.binding = TO_API_FORM(UBOBinding::Light);
	uboBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uboBinding.descriptorCount = 1;
	uboBinding.stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

	vk::DescriptorSetLayoutCreateInfo slci{};
	slci.bindingCount = 1;
	slci.pBindings = &uboBinding;

	{
		vk::ResultValue rv = device.createDescriptorSetLayoutUnique(slci);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("createDescriptorSetLayoutUnique failed: " + vk::to_string(rv.result));
		}
		setLayout_ = std::move(rv.value);
	}


	vk::PipelineLayoutCreateInfo pli{};
	pli.setLayoutCount = 1;
	pli.pSetLayouts = &setLayout_.get();
	{
		vk::ResultValue rv = device.createPipelineLayoutUnique(pli);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("createPipelineLayoutUnique failed: " + vk::to_string(rv.result));
		}
		pipelineLayout_ = std::move(rv.value);
	}

	// depth
	vk::PipelineDepthStencilStateCreateInfo ds{};
	ds.depthTestEnable = vk::True;
	ds.depthWriteEnable = vk::False;
	ds.depthCompareOp = vk::CompareOp::eLessOrEqual;

	vk::PipelineRenderingCreateInfo rendering{};
	rendering.colorAttachmentCount = 1;
	vk::Format format = vk_.getSwapChainImageFormat();
	rendering.pColorAttachmentFormats = &format;
	vk::Format depthFmt = vk_.getDepthFormat();
	rendering.depthAttachmentFormat = depthFmt;

	vk::GraphicsPipelineCreateInfo gp{};
	gp.pNext = &rendering;
	gp.stageCount = 2;
	gp.pStages = stages;
	gp.pVertexInputState = &vi;
	gp.pInputAssemblyState = &ia;
	gp.pViewportState = &vp;  
	gp.pRasterizationState = &rs;
	gp.pMultisampleState = &ms;
	gp.pColorBlendState = &cb;
	gp.pDepthStencilState = &ds;
	gp.pDynamicState = &dyn;
	gp.layout = pipelineLayout_.get();
	gp.renderPass = nullptr;
	gp.subpass = 0;

	vk::ResultValue rv = device.createGraphicsPipelineUnique(nullptr, gp);
	if (rv.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("LightVk: createGraphicsPipelineUnique failed: " + vk::to_string(rv.result));
	}
	pipeline_ = std::move(rv.value);
} // end of createPipeline()

void LightVk::createUBO()
{
	vk::Device device = vk_.getDevice();

	vk::DeviceSize uboSize = sizeof(LightUBO);

	vk::BufferCreateInfo bci{};
	bci.size = uboSize;
	bci.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	bci.sharingMode = vk::SharingMode::eExclusive;

	{
		vk::ResultValue rv = device.createBufferUnique(bci);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("createBufferUnique(UBO) failed: " + vk::to_string(rv.result));
		}
		uboBuffer_ = std::move(rv.value);
	}

	vk::MemoryRequirements req = device.getBufferMemoryRequirements(uboBuffer_.get());

	uint32_t memType = vk_.findMemoryType(
		req.memoryTypeBits,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	vk::MemoryAllocateInfo mai{};
	mai.allocationSize = req.size;
	mai.memoryTypeIndex = memType;

	{
		vk::ResultValue rv = device.allocateMemoryUnique(mai);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("allocateMemoryUnique(UBO) failed: " + vk::to_string(rv.result));
		}
		uboMemory_ = std::move(rv.value);
	}

	{
		vk::Result res = device.bindBufferMemory(uboBuffer_.get(), uboMemory_.get(), 0);
		if (res != vk::Result::eSuccess)
		{
			throw std::runtime_error("bindBufferMemory(UBO) failed: " + vk::to_string(res));
		}
	}
} // end of createUBO()

void LightVk::createDesciptorSet()
{
	vk::Device device = vk_.getDevice();

	vk::DescriptorPoolSize poolSize{};
	poolSize.type = vk::DescriptorType::eUniformBuffer;
	poolSize.descriptorCount = 1;

	vk::DescriptorPoolCreateInfo dpci{};
	dpci.maxSets = 1;
	dpci.poolSizeCount = 1;
	dpci.pPoolSizes = &poolSize;

	auto rvDP = device.createDescriptorPoolUnique(dpci);
	if (rvDP.result != vk::Result::eSuccess)
		throw std::runtime_error("createDescriptorPoolUnique failed: " + vk::to_string(rvDP.result));
	descPool_ = std::move(rvDP.value);

	// Allocate set
	vk::DescriptorSetAllocateInfo dsai{};
	dsai.descriptorPool = descPool_.get();
	dsai.descriptorSetCount = 1;
	auto layout = setLayout_.get();
	dsai.pSetLayouts = &layout;

	auto rvDS = device.allocateDescriptorSets(dsai);
	if (rvDS.result != vk::Result::eSuccess)
		throw std::runtime_error("allocateDescriptorSets failed: " + vk::to_string(rvDS.result));
	descSet_ = rvDS.value[0];

	// Point binding 7 at the buffer
	vk::DescriptorBufferInfo dbi{};
	dbi.buffer = uboBuffer_.get();
	dbi.offset = 0;
	dbi.range = sizeof(LightUBO);

	vk::WriteDescriptorSet write{};
	write.dstSet = descSet_;
	write.dstBinding = TO_API_FORM(UBOBinding::Light);
	write.dstArrayElement = 0;
	write.descriptorType = vk::DescriptorType::eUniformBuffer;
	write.descriptorCount = 1;
	write.pBufferInfo = &dbi;

	device.updateDescriptorSets(1, &write, 0, nullptr);
} // end of createDesciptorSet()