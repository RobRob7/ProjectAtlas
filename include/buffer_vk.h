#ifndef BUFFER_VK_H
#define BUFFER_VK_H

#include <vulkan/vulkan.hpp>

class VulkanMain;

class BufferVk
{
public:
	explicit BufferVk(VulkanMain& vk);
	~BufferVk();

	BufferVk(const BufferVk&) = delete;
	BufferVk& operator=(const BufferVk&) = delete;

	BufferVk(BufferVk&&) noexcept = default;
	BufferVk& operator=(BufferVk&&) noexcept = default;

	void create(
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags properties
	);

	void upload(
		const void* data,
		vk::DeviceSize size,
		vk::DeviceSize offset = 0
	);

	bool valid() const { return static_cast<bool>(buffer_); }
	
	vk::Buffer getBuffer() const { return buffer_.get(); }
	vk::DeviceMemory getMemory() const { return memory_.get(); }

private:
	void destroy();

private:
	VulkanMain& vk_;

	vk::UniqueBuffer buffer_{};
	vk::UniqueDeviceMemory memory_{};

	vk::DeviceSize size_{ 0 };
	vk::MemoryPropertyFlags properties_{};
};

#endif
