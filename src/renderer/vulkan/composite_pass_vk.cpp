#include "composite_pass_vk.h"

#include "frame_context_vk.h"

#include "bindings.h"
#include "shader_vk.h"
#include "vulkan_main.h"
#include "image_vk.h"

#include <vulkan/vulkan.hpp>

#include <cstdint>

//--- PUBLIC ---//
CompositePassVk::CompositePassVk(VulkanMain& vk)
    : vk_(vk),
    hybridColorImage_(vk),
    pipeline_(vk)
{
    descriptorSets_.reserve(vk.getMaxFramesInFlight());
    for (uint32_t i = 0; i < vk_.getMaxFramesInFlight(); ++i)
    {
        descriptorSets_.emplace_back(vk_);
    } // end for
} // end of constructor

CompositePassVk::~CompositePassVk() = default;

void CompositePassVk::init()
{
    shader_ = std::make_unique<ShaderModuleVk>(
        vk_.getDevice(),
        "compositepass/compositepass.vert.spv",
        "compositepass/compositepass.frag.spv"
    );

    createAttachment();
    createDescriptorSet();
    createPipeline();
} // end of init()

void CompositePassVk::resize()
{
    hybridColorImage_.destroy();
    createAttachment();
    refreshInput();
} // end of resize()

void CompositePassVk::render(
    FrameContext& frame,
    float nearPlane,
    float farPlane
)
{
    if (!descriptorSets_[frame.frameIndex].valid() || !pipeline_.valid())
    {
        return;
    }

    vk::CommandBuffer cmd = frame.cmd;

    hybridColorImage_.transitionToColorAttachment(cmd);

    vk::ClearValue clear{};
    clear.color.float32[0] = 0.0f;
    clear.color.float32[1] = 0.0f;
    clear.color.float32[2] = 0.0f;
    clear.color.float32[3] = 1.0f;

    vk::RenderingAttachmentInfo colorAttach{};
    colorAttach.imageView = hybridColorImage_.view();
    colorAttach.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    colorAttach.loadOp = vk::AttachmentLoadOp::eDontCare;
    colorAttach.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttach.clearValue = clear;

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea.offset = vk::Offset2D{ 0, 0 };
    renderingInfo.renderArea.extent = frame.extent;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttach;
    renderingInfo.pDepthAttachment = nullptr;

    cmd.beginRendering(renderingInfo);
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

        vk::DescriptorSet set = descriptorSets_[frame.frameIndex].getSet();

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

    hybridColorImage_.transitionToShaderRead(cmd, vk::ImageAspectFlagBits::eColor);
} // end of render()


//--- PRIVATE ---//
void CompositePassVk::refreshInput()
{
    if (!rasterColor_ || !rasterDepth_ ||
        !rtColor_ || !rtDepth_)
        return;

    for (auto& set : descriptorSets_)
    {
        set.writeCombinedImageSampler(
            TO_API_FORM(CompositePassBinding::RastColorTex),
            rasterColor_->view(),
            rasterColor_->sampler()
        );
        set.writeCombinedImageSampler(
            TO_API_FORM(CompositePassBinding::RastDepthTex),
            rasterDepth_->view(),
            rasterDepth_->sampler()
        );
        set.writeCombinedImageSampler(
            TO_API_FORM(CompositePassBinding::RTColorTex),
            rtColor_->view(),
            rtColor_->sampler()
        );
        set.writeCombinedImageSampler(
            TO_API_FORM(CompositePassBinding::RTDepthTex),
            rtDepth_->view(),
            rtDepth_->sampler()
        );
    } // end for
} // end of refreshInput()

void CompositePassVk::createAttachment()
{
    vk::Extent2D extent = vk_.getSwapChainExtent();

    hybridColorImage_.createImage(
        extent.width,
        extent.height,
        1,
        false,
        vk::SampleCountFlagBits::e1,
        hybridColorFormat_,
        vk::ImageTiling::eOptimal,
        vk::ImageUsageFlagBits::eColorAttachment |
        vk::ImageUsageFlagBits::eSampled,
        vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    hybridColorImage_.createImageView(
        hybridColorFormat_,
        vk::ImageAspectFlagBits::eColor,
        vk::ImageViewType::e2D,
        1
    );

    hybridColorImage_.createSampler(
        vk::Filter::eLinear,
        vk::Filter::eLinear,
        vk::SamplerMipmapMode::eNearest,
        vk::SamplerAddressMode::eClampToEdge,
        false
    );
} // end of createAttachment()

void CompositePassVk::createDescriptorSet()
{
    for (uint32_t i = 0; i < vk_.getMaxFramesInFlight(); ++i)
    {
        vk::DescriptorSetLayoutBinding rastColorBinding{};
        rastColorBinding.binding = TO_API_FORM(CompositePassBinding::RastColorTex);
        rastColorBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        rastColorBinding.descriptorCount = 1;
        rastColorBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        vk::DescriptorSetLayoutBinding rastDepthBinding{};
        rastDepthBinding.binding = TO_API_FORM(CompositePassBinding::RastDepthTex);
        rastDepthBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        rastDepthBinding.descriptorCount = 1;
        rastDepthBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        vk::DescriptorSetLayoutBinding rtColorBinding{};
        rtColorBinding.binding = TO_API_FORM(CompositePassBinding::RTColorTex);
        rtColorBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        rtColorBinding.descriptorCount = 1;
        rtColorBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        vk::DescriptorSetLayoutBinding rtDepthBinding{};
        rtDepthBinding.binding = TO_API_FORM(CompositePassBinding::RTDepthTex);
        rtDepthBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
        rtDepthBinding.descriptorCount = 1;
        rtDepthBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

        descriptorSets_[i].createLayout({
            rastColorBinding,
            rastDepthBinding,
            rtColorBinding,
            rtDepthBinding
            });

        vk::DescriptorPoolSize rastColorPool{};
        rastColorPool.type = vk::DescriptorType::eCombinedImageSampler;
        rastColorPool.descriptorCount = 1;

        vk::DescriptorPoolSize rastDepthPool{};
        rastDepthPool.type = vk::DescriptorType::eCombinedImageSampler;
        rastDepthPool.descriptorCount = 1;

        vk::DescriptorPoolSize rtColorPool{};
        rtColorPool.type = vk::DescriptorType::eCombinedImageSampler;
        rtColorPool.descriptorCount = 1;

        vk::DescriptorPoolSize rtDepthPool{};
        rtDepthPool.type = vk::DescriptorType::eCombinedImageSampler;
        rtDepthPool.descriptorCount = 1;

        descriptorSets_[i].createPool({
            rastColorPool,
            rastDepthPool,
            rtColorPool,
            rtDepthPool
            });
        descriptorSets_[i].allocate();
    } // end for
} // end of createDescriptorSet()

void CompositePassVk::createPipeline()
{
    GraphicsPipelineDescVk desc{};
    desc.vertShader = shader_->vertShader();
    desc.fragShader = shader_->fragShader();

    desc.setLayouts = { descriptorSets_[0].getLayout() };

    desc.colorFormat = hybridColorFormat_;

    desc.cullMode = vk::CullModeFlagBits::eNone;
    desc.frontFace = vk::FrontFace::eClockwise;
    desc.depthTestEnable = false;
    desc.depthWriteEnable = false;

    pipeline_.create(desc);
} // end of createPipeline()