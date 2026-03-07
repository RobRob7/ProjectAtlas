#ifndef DESCRIPTOR_SET_VK_H
#define DESCRIPTOR_SET_VK_H

#include <vulkan/vulkan.hpp>

#include <cstdint>

class VulkanMain;

class DescriptorSetVk
{
public:
	explicit DescriptorSetVk(VulkanMain& vk);
	~DescriptorSetVk();

	DescriptorSetVk(const DescriptorSetVk&) = delete;
	DescriptorSetVk& operator=(const DescriptorSetVk&) = delete;

	DescriptorSetVk(DescriptorSetVk&&) noexcept = default;
	DescriptorSetVk& operator=(DescriptorSetVk&&) noexcept = default;

	void createSingleUniformBuffer(
		uint32_t binding,
		vk::ShaderStageFlags stageFlags,
		vk::Buffer buffer,
		vk::DeviceSize range
	);

	bool valid() const 
	{ 
		return static_cast<bool>(setLayout_) && 
			static_cast<bool>(descPool_) && 
			static_cast<bool>(descSet_); 
	} // end of valid()

	vk::DescriptorSetLayout getLayout() const { return setLayout_.get(); };
	vk::DescriptorSet getSet() const { return descSet_; };

private:
	void destroy();
private:
	VulkanMain& vk_;

	vk::UniqueDescriptorSetLayout setLayout_{};
	vk::UniqueDescriptorPool descPool_{};
	vk::DescriptorSet descSet_{};

};

#endif
