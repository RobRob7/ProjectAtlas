#include "shader_binding_table_vk.h"

#include "vulkan_main.h"

#include <cstdint>
#include <stdexcept>
#include <vector>
#include <cstring>

//--- PUBLIC ---//
ShaderBindingTableVk::ShaderBindingTableVk(VulkanMain& vk)
	: vk_(vk),
	buffer_(vk)
{
} // end of constructor

ShaderBindingTableVk::~ShaderBindingTableVk() = default;

void ShaderBindingTableVk::create(
	vk::Pipeline rtPipeline,
	uint32_t groupCount,
	uint32_t rayGenGroupIndex,
	uint32_t missGroupIndex,
	const std::vector<uint32_t>& hitGroupIndices
)
{
	if (!rtPipeline)
	{
		throw std::runtime_error("ShaderBindingTableVk::create - rtPipeline is NULL!");
	}

	if (groupCount == 0)
	{
		throw std::runtime_error("ShaderBindingTableVk::create - groupCount must be greater than 0!");
	}

	if (hitGroupIndices.empty())
	{
		throw std::runtime_error("ShaderBindingTableVk::create - must provide at least one hit group!");
	}

	if (rayGenGroupIndex >= groupCount ||
		missGroupIndex >= groupCount)
	{
		throw std::runtime_error("ShaderBindingTableVk::create - group index out of range!");
	}

	for (uint32_t hitGroupIndex : hitGroupIndices)
	{
		if (hitGroupIndex >= groupCount)
		{
			throw std::runtime_error("ShaderBindingTableVk::create - hit group index out of range!");
		}
	} // end for

	// RT pipeline properties
	vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rtProps{};
	vk::PhysicalDeviceProperties2 props2{};
	props2.pNext = &rtProps;
	vk_.getPhysicalDevice().getProperties2(&props2);

	const uint32_t handleSize = rtProps.shaderGroupHandleSize;
	const uint32_t handleAlignment = rtProps.shaderGroupHandleAlignment;
	const uint32_t baseAlignment = rtProps.shaderGroupBaseAlignment;

	if (handleSize == 0 || handleAlignment == 0 || baseAlignment == 0)
	{
		throw std::runtime_error("ShaderBindingTableVk::create - invalid RT pipeline properties!");
	}

	auto alignUp = [](vk::DeviceSize value, vk::DeviceSize alignment) -> vk::DeviceSize
		{
			return (value + alignment - 1) & ~(alignment - 1);
		};

	const vk::DeviceSize handleSizeAligned = alignUp(handleSize, handleAlignment);
	const vk::DeviceSize regionStride = alignUp(handleSizeAligned, baseAlignment);

	// get shader group handles from pipeline
    std::vector<uint8_t> handles(static_cast<size_t>(groupCount) * handleSize);

    vk::Result res = vk_.getDevice().getRayTracingShaderGroupHandlesKHR(
        rtPipeline,
        0,
        groupCount,
        handles.size(),
        handles.data()
    );

    if (res != vk::Result::eSuccess)
    {
        throw std::runtime_error(
            "ShaderBindingTableVk::create - getRayTracingShaderGroupHandlesKHR failed: " +
            vk::to_string(res)
        );
    }

    const uint32_t rayGenRecordCount = 1;
    const uint32_t missRecordCount = 1;
    const uint32_t hitRecordCount = static_cast<uint32_t>(hitGroupIndices.size());

    const uint32_t rayGenRecordBase = 0;
    const uint32_t missRecordBase = rayGenRecordBase + rayGenRecordCount;
    const uint32_t hitRecordBase = missRecordBase + missRecordCount;

    const vk::DeviceSize sbtRecordCount =
        rayGenRecordCount + missRecordCount + hitRecordCount;

    const vk::DeviceSize sbtSize = regionStride * sbtRecordCount;

    std::vector<uint8_t> sbtData(static_cast<size_t>(sbtSize), 0);

    auto copyHandleToRecord = [&](uint32_t srcGroupIndex, uint32_t dstRecordIndex)
        {
            const uint8_t* src =
                handles.data() + static_cast<size_t>(srcGroupIndex) * handleSize;

            uint8_t* dst =
                sbtData.data() + static_cast<size_t>(dstRecordIndex) * regionStride;

            std::memcpy(dst, src, handleSize);
        };

    copyHandleToRecord(rayGenGroupIndex, rayGenRecordBase);
    copyHandleToRecord(missGroupIndex, missRecordBase);

    for (uint32_t i = 0; i < hitRecordCount; ++i)
    {
        copyHandleToRecord(hitGroupIndices[i], hitRecordBase + i);
    } // end for

    buffer_.create(
        sbtSize,
        vk::BufferUsageFlagBits::eShaderBindingTableKHR |
        vk::BufferUsageFlagBits::eShaderDeviceAddress,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent,
        true
    );

    buffer_.upload(sbtData.data(), sbtSize);

    const vk::DeviceAddress baseAddress = buffer_.getDeviceAddress();

    rayGenRegion_.deviceAddress = baseAddress + regionStride * rayGenRecordBase;
    rayGenRegion_.stride = regionStride;
    rayGenRegion_.size = regionStride * rayGenRecordCount;

    missRegion_.deviceAddress = baseAddress + regionStride * missRecordBase;
    missRegion_.stride = regionStride;
    missRegion_.size = regionStride * missRecordCount;

    hitRegion_.deviceAddress = baseAddress + regionStride * hitRecordBase;
    hitRegion_.stride = regionStride;
    hitRegion_.size = regionStride * hitRecordCount;

    callableRegion_ = vk::StridedDeviceAddressRegionKHR{};
} // end of create()
