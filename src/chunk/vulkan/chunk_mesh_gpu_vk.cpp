#include "chunk_mesh_gpu_vk.h"

#include "vulkan_main.h"
#include "chunk_mesh_data.h"

#include <cstring>
#include <stdexcept>

//--- PUBLIC ---//
ChunkMeshGPUVk::ChunkMeshGPUVk(VulkanMain& vk)
	: vk_(vk)
{
} // end of constructor

ChunkMeshGPUVk::~ChunkMeshGPUVk() = default;

void ChunkMeshGPUVk::upload(const ChunkMeshData& data)
{
	vk_.waitIdle();

	vk::Device device = vk_.getDevice();

	//--- OPAQUE ---//
	opaqueIndexCount_ = static_cast<uint32_t>(data.opaqueIndices.size());
	if (!data.opaqueVertices.empty() && !data.opaqueIndices.empty())
	{
		vk::DeviceSize vbSize = sizeof(Vertex) * data.opaqueVertices.size();
		vk::DeviceSize ibSize = sizeof(uint32_t) * data.opaqueIndices.size();

		// staging VB
		vk_.createBuffer(
			vbSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			opaqueVB_.get(), opaqueVBMem_.get());
		{
			auto rv = device.mapMemory(opaqueVBMem_.get(), 0, vbSize);
			if (rv.result != vk::Result::eSuccess)
			{
				throw std::runtime_error("mapMemory failed: " + vk::to_string(rv.result));
			}
			std::memcpy(rv.value, data.opaqueVertices.data(), static_cast<size_t>(vbSize));
		}
		device.unmapMemory(opaqueVBMem_.get());

		// staging IB
		vk_.createBuffer(
			ibSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			opaqueIB_.get(), opaqueIBMem_.get());
		{
			auto rv = device.mapMemory(opaqueIBMem_.get(), 0, vbSize);
			if (rv.result != vk::Result::eSuccess)
			{
				throw std::runtime_error("mapMemory failed: " + vk::to_string(rv.result));
			}
			std::memcpy(rv.value, data.opaqueIndices.data(), static_cast<size_t>(vbSize));
		}
		device.unmapMemory(opaqueIBMem_.get());
	}
	else
	{
		opaqueIndexCount_ = 0;
	}


	//--- WATER ---//
	waterIndexCount_ = static_cast<uint32_t>(data.waterIndices.size());
	if (!data.waterVertices.empty() && !data.waterIndices.empty())
	{
		vk::DeviceSize vbSize = sizeof(Vertex) * data.waterVertices.size();
		vk::DeviceSize ibSize = sizeof(uint32_t) * data.waterIndices.size();

		// staging VB
		vk_.createBuffer(
			vbSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			waterVB_.get(), waterVBMem_.get());
		{
			auto rv = device.mapMemory(waterVBMem_.get(), 0, vbSize);
			if (rv.result != vk::Result::eSuccess)
			{
				throw std::runtime_error("mapMemory failed: " + vk::to_string(rv.result));
			}
			std::memcpy(rv.value, data.waterVertices.data(), static_cast<size_t>(vbSize));
		}
		device.unmapMemory(waterVBMem_.get());

		// staging IB
		vk_.createBuffer(
			ibSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			waterIB_.get(), waterIBMem_.get());
		{
			auto rv = device.mapMemory(waterIBMem_.get(), 0, vbSize);
			if (rv.result != vk::Result::eSuccess)
			{
				throw std::runtime_error("mapMemory failed: " + vk::to_string(rv.result));
			}
			std::memcpy(rv.value, data.waterIndices.data(), static_cast<size_t>(vbSize));
		}
		device.unmapMemory(waterIBMem_.get());
	}
	else
	{
		waterIndexCount_ = 0;
	}
} // end of upload()

void ChunkMeshGPUVk::drawOpaque()
{
	//auto cmd = static_cast<VkCommandBuffer>(ctx.backendCmd);

	//vk_.

	//if (!cmd || opaqueIndexCount_ == 0) return;

	//vk::DeviceSize offset = 0;

	//vkCmdBindVertexBuffers(cmd, 0, 1, &opaqueVB_, &offset);
	//vkCmdBindIndexBuffer(cmd, opaqueIB_, 0, VK_INDEX_TYPE_UINT32);
	//vkCmdDrawIndexed(cmd, opaqueIndexCount_, 1, 0, 0, 0);
} // end of drawOpaque()

void ChunkMeshGPUVk::drawWater()
{
	//auto cmd = static_cast<VkCommandBuffer>(ctx.backendCmd);

	//if (!cmd || waterIndexCount_ == 0) return;

	//VkDeviceSize offset = 0;
	//vkCmdBindVertexBuffers(cmd, 0, 1, &waterVB_, &offset);
	//vkCmdBindIndexBuffer(cmd, waterIB_, 0, VK_INDEX_TYPE_UINT32);
	//vkCmdDrawIndexed(cmd, waterIndexCount_, 1, 0, 0, 0);
} // end of drawWater()
