#include "present_pass_vk.h"

#include "frame_context_vk.h"

#include "bindings.h"
#include "shader_vk.h"
#include "vulkan_main.h"
#include "image_vk.h"

#include <vulkan/vulkan.hpp>

//--- PUBLIC ---//
PresentPassVk::PresentPassVk(VulkanMain& vk)
	: vk_(vk),
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

void PresentPassVk::setInput(ImageVk& input)
{
    inputImage_ = &input;
} // end of setInput()

void PresentPassVk::render(
    vk::CommandBuffer cmd,
    FrameContext& frame
)
{
    refreshInput();

    if (!inputImage_ || !descriptorSet_.valid() || !pipeline_.valid())
    {
        return;
    }

    vk::ClearValue clear{};
    clear.color.float32[0] = 0.0f;
    clear.color.float32[1] = 0.0f;
    clear.color.float32[2] = 0.0f;
    clear.color.float32[3] = 1.0f;

    vk::RenderingAttachmentInfo presentColorAttach{};
    presentColorAttach.imageView = frame.colorImageView;
    presentColorAttach.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    presentColorAttach.loadOp = vk::AttachmentLoadOp::eDontCare;
    presentColorAttach.storeOp = vk::AttachmentStoreOp::eStore;
    presentColorAttach.clearValue = clear;

    vk::RenderingInfo presentRenderingInfo{};
    presentRenderingInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    presentRenderingInfo.renderArea.extent = frame.extent;
    presentRenderingInfo.layerCount = 1;
    presentRenderingInfo.colorAttachmentCount = 1;
    presentRenderingInfo.pColorAttachments = &presentColorAttach;
    presentRenderingInfo.pDepthAttachment = nullptr;

    cmd.beginRendering(presentRenderingInfo);
    {
        vk::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(frame.extent.width);
        viewport.height = static_cast<float>(frame.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        cmd.setViewport(0, 1, &viewport);

        vk::Rect2D scissor{};
        scissor.offset = vk::Offset2D{ 0, 0 };
        scissor.extent = frame.extent;
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
} // end of render()


//--- PRIVATE ---//
void PresentPassVk::refreshInput()
{
    if (!inputImage_)
        return;

    if (boundInputImage_ == inputImage_ &&
        inputGeneration_ == inputImage_->generation())
    {
        return;
    }

    descriptorSet_.writeCombinedImageSampler(
        TO_API_FORM(PresentPassBinding::ForwardColorTex),
        inputImage_->view(),
        inputImage_->sampler()
    );

    boundInputImage_ = inputImage_;
    inputGeneration_ = inputImage_->generation();
} // end of refreshInput()

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

    desc.colorFormat = vk_.getSwapChainImageFormat();
    desc.depthFormat = vk::Format::eUndefined;

    desc.cullMode = vk::CullModeFlagBits::eNone;
    desc.frontFace = vk::FrontFace::eClockwise;
    desc.depthTestEnable = false;
    desc.depthWriteEnable = false;

    pipeline_.create(desc);
} // end of createPipeline()