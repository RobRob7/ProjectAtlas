#ifndef FRAME_CONTEXT_H
#define FRAME_CONTEXT_H

#include <vulkan/vulkan.hpp>

struct FrameContext
{
    vk::CommandBuffer cmd{};

    vk::Extent2D extent{};

    uint32_t frameIndex = 0;
    uint32_t imageIndex = 0;

    vk::Image colorImage{};
    vk::ImageView colorImageView{};
    vk::ImageLayout colorLayout{ vk::ImageLayout::eUndefined };

    vk::Image depthImage{};
    vk::ImageView depthImageView{};
    vk::ImageLayout depthLayout{ vk::ImageLayout::eUndefined };

    vk::Format colorFormat{ vk::Format::eUndefined };
    vk::Format depthFormat{ vk::Format::eUndefined };
};

#endif
