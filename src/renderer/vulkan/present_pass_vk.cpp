#include "present_pass_vk.h"

#include "bindings.h"
#include "shader_vk.h"
#include "vulkan_main.h"

#include <vulkan/vulkan.hpp>

//--- PUBLIC ---//
PresentPassVk::PresentPassVk(VulkanMain& vk, ImageVk& inputImage)
	: vk_(vk),
	inputImage_(inputImage),
	descriptorSet_(vk),
	pipeline_(vk)
{
} // end of constructor

PresentPassVk::~PresentPassVk() = default;

void PresentPassVk::init()
{
    shader_ = std::make_unique<ShaderModuleVk>(
        vk_.getDevice(),
        "presentpass/present.vert.spv",
        "presentpass/present.frag.spv"
    );

    createDescriptorSet();
    createPipeline();
} // end of init()

void PresentPassVk::refreshInput()
{
    descriptorSet_.writeCombinedImageSampler(
        TO_API_FORM(PresentPassBinding::ForwardColorTex),
        inputImage_.view(),
        inputImage_.sampler()
    );
} // end of refreshInput()

void PresentPassVk::render(vk::CommandBuffer cmd)
{
    if (!descriptorSet_.valid() || !pipeline_.valid())
    {
        return;
    }

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
} // end of render()


//--- PRIVATE ---//
void PresentPassVk::createDescriptorSet()
{
    vk::DescriptorSetLayoutBinding inputBinding{};
    inputBinding.binding = TO_API_FORM(PresentPassBinding::ForwardColorTex);
    inputBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
    inputBinding.descriptorCount = 1;
    inputBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

    descriptorSet_.createLayout({ inputBinding });

    vk::DescriptorPoolSize inputPool{};
    inputPool.type = vk::DescriptorType::eCombinedImageSampler;
    inputPool.descriptorCount = 1;

    descriptorSet_.createPool({ inputPool }, 1);
    descriptorSet_.allocate();
} // end of createDescriptorSet()

void PresentPassVk::createPipeline()
{
    GraphicsPipelineDescVk desc{};
    desc.vertShader = shader_->vertShader();
    desc.fragShader = shader_->fragShader();

    desc.setLayouts = { descriptorSet_.getLayout() };

    // output is swapchain image
    desc.colorFormat = vk_.getSwapChainImageFormat();
    desc.depthFormat = vk::Format::eUndefined;

    desc.cullMode = vk::CullModeFlagBits::eNone;
    desc.frontFace = vk::FrontFace::eClockwise;
    desc.depthTestEnable = false;
    desc.depthWriteEnable = false;

    // no vertex input needed for fullscreen triangle
    pipeline_.create(desc);
} // end of createPipeline()