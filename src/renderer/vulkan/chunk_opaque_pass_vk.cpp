#include "chunk_opaque_pass_vk.h"

#include "render_inputs.h"
#include "chunk_manager.h"
#include "i_chunk_mesh_gpu.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <stdexcept>
#include <fstream>

struct ChunkFrameUBO
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec2 screenSize;
    float ambientStrength;
    float _pad0;
};

struct ChunkPush
{
    glm::vec3 chunkOrigin;
    float _pad0;
};

//--- HELPER ---//
static std::vector<uint8_t> readFileBytes(const char* path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) throw std::runtime_error("failed to open file");

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data((size_t)size);
    if (!file.read((char*)data.data(), size)) throw std::runtime_error("failed to read file");
    return data;
} // end of readFileBytes()


//--- PUBLIC ---//
ChunkOpaquePassVk::ChunkOpaquePassVk(VulkanMain& vk) : vk_(vk) {}
ChunkOpaquePassVk::~ChunkOpaquePassVk() { destroy(); }

void ChunkOpaquePassVk::init(VkFormat swapchainFormat)
{
    createDescriptorSetLayout();
    createPerFrameUBO();
    createDescriptorPoolAndSets();

    // TODO: create atlasView_/atlasSampler_ and call updateDescriptorSets()
    // For now you can still draw if your shader doesn’t sample the atlas.
    // If shader *does* sample, you need to create these before updating.

    createPipeline(swapchainFormat);
}

void ChunkOpaquePassVk::onSwapchainRecreated(VkFormat swapchainFormat)
{
    VkDevice dev = vk_.device();
    if (opaquePipe_) vkDestroyPipeline(dev, opaquePipe_, nullptr);
    opaquePipe_ = VK_NULL_HANDLE;

    createPipeline(swapchainFormat);
}

void ChunkOpaquePassVk::renderOpaque(
    const RenderInputs& in,
    VkFrameContext& frame,
    const glm::mat4& view,
    const glm::mat4& proj)
{
    // 1) build list (same as GL)
    ChunkDrawList list;
    in.world->buildOpaqueDrawList(view, proj, list);

    VkCommandBuffer cmd = frame.cmd;
    if (!cmd) return;

    // 2) update per-frame UBO for *this* frame-in-flight
    const uint32_t fi = frame.frameIndex;

    ChunkFrameUBO ubo{};
    ubo.view = view;
    ubo.proj = proj;
    ubo.screenSize = glm::vec2((float)frame.extent.width, (float)frame.extent.height);
    ubo.ambientStrength = in.world ? in.world->getAmbientStrength() : 0.2f;

    void* mapped = nullptr;
    vkMapMemory(vk_.device(), uboMem_[fi], 0, sizeof(ChunkFrameUBO), 0, &mapped);
    std::memcpy(mapped, &ubo, sizeof(ChunkFrameUBO));
    vkUnmapMemory(vk_.device(), uboMem_[fi]);

    // 3) layout -> color attachment
    transitionSwapchain(cmd, frame.imageIndex, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // 4) begin dynamic rendering to swapchain image view
    beginRendering(cmd, frame.swapchainImageView, frame.extent);

    // 5) bind pipeline
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, opaquePipe_);

    // viewport/scissor (dynamic)
    VkViewport vp{};
    vp.x = 0;
    vp.y = 0;
    vp.width = (float)frame.extent.width;
    vp.height = (float)frame.extent.height;
    vp.minDepth = 0.f;
    vp.maxDepth = 1.f;
    vkCmdSetViewport(cmd, 0, 1, &vp);

    VkRect2D sc{};
    sc.offset = { 0,0 };
    sc.extent = frame.extent;
    vkCmdSetScissor(cmd, 0, 1, &sc);

    // 6) bind descriptor set (UBO + atlas sampler)
    vkCmdBindDescriptorSets(
        cmd,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeLayout_,
        0, 1,
        &descSet_[fi],
        0, nullptr);

    // 7) loop chunks: push origin + draw
    DrawContext ctx{};
    ctx.backendCmd = reinterpret_cast<void*>(cmd);

    for (const auto& item : list.items)
    {
        if (!item.validOpaque()) continue;

        ChunkPush push{};
        push.chunkOrigin = item.chunkOrigin;

        vkCmdPushConstants(
            cmd, pipeLayout_,
            VK_SHADER_STAGE_VERTEX_BIT,
            0, sizeof(ChunkPush),
            &push);

        item.gpu->drawOpaque(ctx);
    }

    // 8) end rendering
    endRendering(cmd);

    // 9) layout -> present
    transitionSwapchain(cmd, frame.imageIndex, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}


//--- PRIVATE ---//
void ChunkOpaquePassVk::createDescriptorSetLayout()
{
    // set=0 binding=0 : UBO
    VkDescriptorSetLayoutBinding uboBinding{};
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    // set=0 binding=1 : atlas sampler
    VkDescriptorSetLayoutBinding samplerBinding{};
    samplerBinding.binding = 1;
    samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerBinding.descriptorCount = 1;
    samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[] = { uboBinding, samplerBinding };

    VkDescriptorSetLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 2;
    info.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(vk_.device(), &info, nullptr, &setLayout_) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor set layout");
}

void ChunkOpaquePassVk::createPerFrameUBO()
{
    const uint32_t frames = vk_.maxFramesInFlight();
    ubo_.resize(frames, VK_NULL_HANDLE);
    uboMem_.resize(frames, VK_NULL_HANDLE);

    for (uint32_t i = 0; i < frames; ++i)
    {
        vk_.createBuffer(
            sizeof(ChunkFrameUBO),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            ubo_[i],
            uboMem_[i]);
    }
}

void ChunkOpaquePassVk::createDescriptorPoolAndSets()
{
    const uint32_t frames = vk_.maxFramesInFlight();

    VkDescriptorPoolSize poolSizes[2]{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = frames;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = frames;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = frames;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;

    if (vkCreateDescriptorPool(vk_.device(), &poolInfo, nullptr, &descPool_) != VK_SUCCESS)
        throw std::runtime_error("failed to create descriptor pool");

    std::vector<VkDescriptorSetLayout> layouts(frames, setLayout_);

    VkDescriptorSetAllocateInfo alloc{};
    alloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc.descriptorPool = descPool_;
    alloc.descriptorSetCount = frames;
    alloc.pSetLayouts = layouts.data();

    descSet_.resize(frames, VK_NULL_HANDLE);
    if (vkAllocateDescriptorSets(vk_.device(), &alloc, descSet_.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate descriptor sets");

    updateDescriptorSets();
}

void ChunkOpaquePassVk::updateDescriptorSets()
{
    const uint32_t frames = vk_.maxFramesInFlight();

    for (uint32_t i = 0; i < frames; ++i)
    {
        VkDescriptorBufferInfo bi{};
        bi.buffer = ubo_[i];
        bi.offset = 0;
        bi.range = sizeof(ChunkFrameUBO);

        VkWriteDescriptorSet writes[2]{};

        writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[0].dstSet = descSet_[i];
        writes[0].dstBinding = 0;
        writes[0].descriptorCount = 1;
        writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[0].pBufferInfo = &bi;

        // NOTE: if atlasView_/atlasSampler_ aren’t created yet, don’t write binding=1
        // and make your fragment shader not sample it yet.
        VkDescriptorImageInfo ii{};
        ii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        ii.imageView = atlasView_;
        ii.sampler = atlasSampler_;

        writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[1].dstSet = descSet_[i];
        writes[1].dstBinding = 1;
        writes[1].descriptorCount = 1;
        writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[1].pImageInfo = &ii;

        // If you haven’t created atlasView_/atlasSampler_ yet, do only writes[0]
        if (atlasView_ && atlasSampler_)
        {
            vkUpdateDescriptorSets(vk_.device(), 2, writes, 0, nullptr);
        }
        else
        {
            vkUpdateDescriptorSets(vk_.device(), 1, writes, 0, nullptr);
        }
    }
}

// ------------------ pipeline ------------------
VkShaderModule ChunkOpaquePassVk::createShaderModule(const std::vector<uint8_t>& bytes)
{
    VkShaderModuleCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    info.codeSize = bytes.size();
    info.pCode = reinterpret_cast<const uint32_t*>(bytes.data());

    VkShaderModule mod = VK_NULL_HANDLE;
    if (vkCreateShaderModule(vk_.device(), &info, nullptr, &mod) != VK_SUCCESS)
        throw std::runtime_error("failed to create shader module");
    return mod;
}

void ChunkOpaquePassVk::createPipeline(VkFormat swapchainFormat)
{
    // --- pipeline layout (set=0 + push constants) ---
    VkPushConstantRange pcr{};
    pcr.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pcr.offset = 0;
    pcr.size = sizeof(ChunkPush);

    VkPipelineLayoutCreateInfo pl{};
    pl.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pl.setLayoutCount = 1;
    pl.pSetLayouts = &setLayout_;
    pl.pushConstantRangeCount = 1;
    pl.pPushConstantRanges = &pcr;

    if (!pipeLayout_)
    {
        if (vkCreatePipelineLayout(vk_.device(), &pl, nullptr, &pipeLayout_) != VK_SUCCESS)
            throw std::runtime_error("failed to create pipeline layout");
    }

    // --- shaders (you need SPIR-V paths) ---
    auto vertBytes = readFileBytes("shaders/chunk.vert.spv");
    auto fragBytes = readFileBytes("shaders/chunk.frag.spv");
    VkShaderModule vert = createShaderModule(vertBytes);
    VkShaderModule frag = createShaderModule(fragBytes);

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vert;
    stages[0].pName = "main";

    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = frag;
    stages[1].pName = "main";

    // --- vertex input ---
    // TODO: replace these with your actual Vertex binding/attributes.
    VkPipelineVertexInputStateCreateInfo vi{};
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.vertexBindingDescriptionCount = 0;
    vi.vertexAttributeDescriptionCount = 0;

    VkPipelineInputAssemblyStateCreateInfo ia{};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo vp{};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rs{};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.f;

    VkPipelineMultisampleStateCreateInfo ms{};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // no depth yet (keep simple)
    VkPipelineDepthStencilStateCreateInfo ds{};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_FALSE;
    ds.depthWriteEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState cbAttach{};
    cbAttach.blendEnable = VK_FALSE;
    cbAttach.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo cb{};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &cbAttach;

    VkDynamicState dynStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn{};
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = 2;
    dyn.pDynamicStates = dynStates;

    // dynamic rendering info (no render pass)
    VkPipelineRenderingCreateInfo pr{};
    pr.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pr.colorAttachmentCount = 1;
    pr.pColorAttachmentFormats = &swapchainFormat;

    VkGraphicsPipelineCreateInfo gp{};
    gp.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gp.pNext = &pr;
    gp.stageCount = 2;
    gp.pStages = stages;
    gp.pVertexInputState = &vi;
    gp.pInputAssemblyState = &ia;
    gp.pViewportState = &vp;
    gp.pRasterizationState = &rs;
    gp.pMultisampleState = &ms;
    gp.pDepthStencilState = &ds;
    gp.pColorBlendState = &cb;
    gp.pDynamicState = &dyn;
    gp.layout = pipeLayout_;

    if (vkCreateGraphicsPipelines(vk_.device(), VK_NULL_HANDLE, 1, &gp, nullptr, &opaquePipe_) != VK_SUCCESS)
        throw std::runtime_error("failed to create opaque pipeline");

    vkDestroyShaderModule(vk_.device(), vert, nullptr);
    vkDestroyShaderModule(vk_.device(), frag, nullptr);
}

// ------------------ dynamic rendering ------------------
void ChunkOpaquePassVk::beginRendering(VkCommandBuffer cmd, VkImageView targetView, VkExtent2D extent)
{
    VkRenderingAttachmentInfo color{};
    color.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    color.imageView = targetView;
    color.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkClearValue clear{};
    clear.color = { {0.f, 0.f, 0.f, 1.f} };
    color.clearValue = clear;

    VkRenderingInfo info{};
    info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    info.renderArea.offset = { 0,0 };
    info.renderArea.extent = extent;
    info.layerCount = 1;
    info.colorAttachmentCount = 1;
    info.pColorAttachments = &color;

    vkCmdBeginRendering(cmd, &info);
}

void ChunkOpaquePassVk::endRendering(VkCommandBuffer cmd)
{
    vkCmdEndRendering(cmd);
}

// ------------------ layout transitions ------------------
void ChunkOpaquePassVk::transitionSwapchain(VkCommandBuffer cmd, uint32_t imageIndex, VkImageLayout newLayout)
{
    VkImageLayout oldLayout = vk_.swapchainLayout(imageIndex);
    if (oldLayout == newLayout) return;

    VkImageMemoryBarrier2 b{};
    b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    b.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    b.srcAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
    b.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    b.dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
    b.oldLayout = oldLayout;
    b.newLayout = newLayout;
    b.image = vk_.swapChainImages()[imageIndex];
    b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    b.subresourceRange.levelCount = 1;
    b.subresourceRange.layerCount = 1;

    VkDependencyInfo dep{};
    dep.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dep.imageMemoryBarrierCount = 1;
    dep.pImageMemoryBarriers = &b;

    vkCmdPipelineBarrier2(cmd, &dep);
    vk_.setSwapchainLayout(imageIndex, newLayout);
}

// ------------------ cleanup ------------------
void ChunkOpaquePassVk::destroy()
{
    VkDevice dev = vk_.device();

    if (opaquePipe_) vkDestroyPipeline(dev, opaquePipe_, nullptr);
    opaquePipe_ = VK_NULL_HANDLE;

    if (pipeLayout_) vkDestroyPipelineLayout(dev, pipeLayout_, nullptr);
    pipeLayout_ = VK_NULL_HANDLE;

    if (descPool_) vkDestroyDescriptorPool(dev, descPool_, nullptr);
    descPool_ = VK_NULL_HANDLE;

    if (setLayout_) vkDestroyDescriptorSetLayout(dev, setLayout_, nullptr);
    setLayout_ = VK_NULL_HANDLE;

    for (size_t i = 0; i < ubo_.size(); ++i)
    {
        if (ubo_[i]) vkDestroyBuffer(dev, ubo_[i], nullptr);
        if (uboMem_[i]) vkFreeMemory(dev, uboMem_[i], nullptr);
    }
    ubo_.clear();
    uboMem_.clear();
    descSet_.clear();

    // atlas resources if you create them later
    if (atlasSampler_) vkDestroySampler(dev, atlasSampler_, nullptr);
    if (atlasView_) vkDestroyImageView(dev, atlasView_, nullptr);
    if (atlasImage_) vkDestroyImage(dev, atlasImage_, nullptr);
    if (atlasMem_) vkFreeMemory(dev, atlasMem_, nullptr);

    atlasSampler_ = VK_NULL_HANDLE;
    atlasView_ = VK_NULL_HANDLE;
    atlasImage_ = VK_NULL_HANDLE;
    atlasMem_ = VK_NULL_HANDLE;
}