#ifndef UTILS_VK_H
#define UTILS_VK_H

#include "vulkan_main.h"

#include <vulkan/vulkan.hpp>

#include <cstdint>

namespace VkUtils
{
	void TransitionImageLayoutImmediate(
		VulkanMain& vk,
		vk::Image image,
		vk::ImageAspectFlags aspectMask,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		uint32_t layers,
		uint32_t mipLevels
	);

	void CopyBufferToImageImmediate(
		VulkanMain& vk,
		vk::Buffer buffer,
		vk::Image image,
		uint32_t width,
		uint32_t height,
		uint32_t layers
	);

	void TransitionImageLayout(
		vk::CommandBuffer cmd,
		vk::Image image,
		vk::ImageAspectFlags aspectMask,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		uint32_t layers,
		uint32_t mipLevels
	);
}

#endif
