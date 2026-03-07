#include "light_vk.h"

#include "vulkan_main.h"
#include "shader_vk.h"

#include "ubo_bindings.h"

#include <vulkan/vulkan.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <cstddef>
#include <array>
#include <cassert>
#include <cstdint>

using namespace Light_Constants;

//--- PUBLIC ---//
LightVk::LightVk(VulkanMain& vk, const glm::vec3& pos, const glm::vec3& color)
	: vk_(vk),
	uboBuffer_(vk),
	vertexBuffer_(vk),
	descriptorSet_(vk),
	pipeline_(vk),
	position_(pos)
{
	setColor(color);
} // end of constructor

LightVk::~LightVk() = default;

void LightVk::init()
{
	shader_ = std::make_unique<ShaderModuleVk>(vk_.getDevice(), "light/light.vert.spv", "light/light.frag.spv");

	createVertexBuffer();
	createUBO();
	createDescriptorSet();
	createPipeline();
} // end of init()

void LightVk::render(const RenderContext& ctx, const glm::mat4& view, const glm::mat4& proj)
{
	assert(ctx.backend == RenderContext::Backend::Vulkan && "Must be Vulkan render context!");

	if (!descriptorSet_.valid() || !uboBuffer_.valid() || !vertexBuffer_.valid() || !pipeline_.valid()) return;

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

	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_.getPipeline());

	vk::Buffer vertBuffer = vertexBuffer_.getBuffer();
	vk::DeviceSize offset = 0;
	cmd.bindVertexBuffers(0, 1, &vertBuffer, &offset);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), position_);

	LightUBO ubo{};
	ubo.model = model;
	ubo.view = view;
	ubo.proj = proj;
	ubo.color = glm::vec4(color_, 1.0f);

	uboBuffer_.upload(&ubo, sizeof(LightUBO));

	vk::DescriptorSet descSet = descriptorSet_.getSet();
	cmd.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		pipeline_.getLayout(),
		0,
		1, &descSet,
		0, nullptr
	);

	cmd.draw(vertexCount_, 1, 0, 0);
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

	vertexCount_ = static_cast<uint32_t>(verts.size());

	const vk::DeviceSize bufferSize = sizeof(VertexLight) * verts.size();
	vertexBuffer_.create(
		bufferSize,
		vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);

	vertexBuffer_.upload(verts.data(), bufferSize);
} // end of createVertexBuffer()

void LightVk::createUBO()
{
	uboBuffer_.create(
		sizeof(LightUBO),
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
} // end of createUBO()

void LightVk::createDescriptorSet()
{
	descriptorSet_.createSingleUniformBuffer(
		TO_API_FORM(UBOBinding::Light),
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
		uboBuffer_.getBuffer(),
		sizeof(LightUBO)
	);
} // end of createDescriptorSet()

void LightVk::createPipeline()
{
	vk::VertexInputBindingDescription binding{};
	binding.binding = 0;
	binding.stride = sizeof(VertexLight);
	binding.inputRate = vk::VertexInputRate::eVertex;

	vk::VertexInputAttributeDescription attr{};
	attr.location = 0;
	attr.binding = 0;
	attr.format = vk::Format::eR32G32B32Sfloat;
	attr.offset = offsetof(VertexLight, pos);

	GraphicsPipelineDescVk desc{};
	desc.vertShader = shader_->vertShader();
	desc.fragShader = shader_->fragShader();

	desc.setLayouts = { descriptorSet_.getLayout() };

	desc.vertexBinding = binding;
	desc.vertexAttributes = { attr };

	desc.colorFormat = vk_.getSwapChainImageFormat();
	desc.depthFormat = vk_.getDepthFormat();

	desc.cullMode = vk::CullModeFlagBits::eBack;
	desc.frontFace = vk::FrontFace::eCounterClockwise;
	desc.depthTestEnable = true;
	desc.depthWriteEnable = false;
	desc.depthCompareOp = vk::CompareOp::eLessOrEqual;

	pipeline_.create(desc);
} // end of createPipeline()
