#include "descriptor_set_vk.h"

#include "vulkan_main.h"

#include <vulkan/vulkan.hpp>

#include <stdexcept>
#include <cstdint>

//--- PUBLIC ---//
DescriptorSetVk::DescriptorSetVk(VulkanMain& vk)
	: vk_(vk)
{
} // end of constructor

DescriptorSetVk::~DescriptorSetVk() = default;

void DescriptorSetVk::createSingleUniformBuffer(
	uint32_t binding,
	vk::ShaderStageFlags stageFlags,
	vk::Buffer buffer,
	vk::DeviceSize range
)
{
	destroy();

	vk::Device device = vk_.getDevice();

	// layout
	vk::DescriptorSetLayoutBinding uboBinding{};
	uboBinding.binding = binding;
	uboBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
	uboBinding.descriptorCount = 1;
	uboBinding.stageFlags = stageFlags;

	vk::DescriptorSetLayoutCreateInfo slci{};
	slci.bindingCount = 1;
	slci.pBindings = &uboBinding;

	{
		vk::ResultValue rv = device.createDescriptorSetLayoutUnique(slci);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error(
				"DescriptorSetVk::createSingleUniformBuffer - createDescriptorSetLayoutUnique failed: " +
				vk::to_string(rv.result)
			);
		}
		setLayout_ = std::move(rv.value);
	}

	// pool
	vk::DescriptorPoolSize poolSize{};
	poolSize.type = vk::DescriptorType::eUniformBuffer;
	poolSize.descriptorCount = 1;

	vk::DescriptorPoolCreateInfo dpci{};
	dpci.maxSets = 1;
	dpci.poolSizeCount = 1;
	dpci.pPoolSizes = &poolSize;

	{
		vk::ResultValue rv = device.createDescriptorPoolUnique(dpci);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error(
				"DescriptorSetVk::createSingleUniformBuffer - createDescriptorPoolUnique failed: " +
				vk::to_string(rv.result)
			);
		}
		descPool_ = std::move(rv.value);
	}

	// allocate set
	vk::DescriptorSetAllocateInfo dsai{};
	dsai.descriptorPool = descPool_.get();
	dsai.descriptorSetCount = 1;

	vk::DescriptorSetLayout layoutHandle = setLayout_.get();
	dsai.pSetLayouts = &layoutHandle;

	{
		vk::ResultValue rv = device.allocateDescriptorSets(dsai);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error(
				"DescriptorSetVk::createSingleUniformBuffer - allocateDescriptorSets failed: " +
				vk::to_string(rv.result)
			);
		}
		descSet_ = rv.value[0];
	}

	// write descriptor
	vk::DescriptorBufferInfo dbi{};
	dbi.buffer = buffer;
	dbi.offset = 0;
	dbi.range = range;

	vk::WriteDescriptorSet write{};
	write.dstSet = descSet_;
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorType = vk::DescriptorType::eUniformBuffer;
	write.descriptorCount = 1;
	write.pBufferInfo = &dbi;

	device.updateDescriptorSets(1, &write, 0, nullptr);
} // end of createSingleUniformBuffer()


//--- PRIVATE ---//
void DescriptorSetVk::destroy()
{
	setLayout_.reset();
	descPool_.reset();
	descSet_ = vk::DescriptorSet{};
} // end of destroy()