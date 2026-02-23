#include "chunk_mesh_gpu_vk.h"

#include "vulkan_main.h"
#include "chunk_mesh_data.h"

#include <cstring>

//--- PUBLIC ---//
ChunkMeshGPUVk::ChunkMeshGPUVk(VulkanMain& vk)
	: vk_(vk)
{
} // end of constructor

ChunkMeshGPUVk::~ChunkMeshGPUVk()
{
	destroyBuffers();
} // end of destructor

void ChunkMeshGPUVk::upload(const ChunkMeshData& data)
{
	vk_.waitIdle();
	destroyBuffers();

	VkDevice device = vk_.device();

	//--- OPAQUE ---//
	opaqueIndexCount_ = static_cast<uint32_t>(data.opaqueIndices.size());
	if (!data.opaqueVertices.empty() && !data.opaqueIndices.empty())
	{
		VkDeviceSize vbSize = sizeof(Vertex) * data.opaqueVertices.size();
		VkDeviceSize ibSize = sizeof(uint32_t) * data.opaqueIndices.size();

		// staging VB
		VkBuffer stagingVB{};
		VkDeviceMemory stagingVBMem{};
		vk_.createBuffer(
			vbSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingVB, stagingVBMem);

		void* mapped = nullptr;
		vkMapMemory(device, stagingVBMem, 0, vbSize, 0, &mapped);
		std::memcpy(mapped, data.opaqueVertices.data(), static_cast<size_t>(vbSize));
		vkUnmapMemory(device, stagingVBMem);

		// staging IB
		VkBuffer stagingIB{};
		VkDeviceMemory stagingIBMem{};
		vk_.createBuffer(
			ibSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingIB, stagingIBMem);

		vkMapMemory(device, stagingIBMem, 0, ibSize, 0, &mapped);
		std::memcpy(mapped, data.opaqueIndices.data(), static_cast<size_t>(ibSize));
		vkUnmapMemory(device, stagingIBMem);

		// device local VB/IB
		vk_.createBuffer(
			vbSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			opaqueVB_, opaqueVBMem_);

		vk_.createBuffer(
			ibSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			opaqueIB_, opaqueIBMem_);

		// copy staging to device local
		vk_.copyBuffer(stagingVB, opaqueVB_, vbSize);
		vk_.copyBuffer(stagingIB, opaqueIB_, ibSize);

		// destroy staging
		vkDestroyBuffer(device, stagingVB, nullptr);
		vkFreeMemory(device, stagingVBMem, nullptr);
		vkDestroyBuffer(device, stagingIB, nullptr);
		vkFreeMemory(device, stagingIBMem, nullptr);
	}
	else
	{
		opaqueIndexCount_ = 0;
	}


	//--- WATER ---//
	waterIndexCount_ = static_cast<uint32_t>(data.waterIndices.size());
	if (!data.waterVertices.empty() && !data.waterIndices.empty())
	{
		VkDeviceSize vbSize = sizeof(Vertex) * data.waterVertices.size();
		VkDeviceSize ibSize = sizeof(uint32_t) * data.waterIndices.size();

		// staging VB
		VkBuffer stagingVB{};
		VkDeviceMemory stagingVBMem{};
		vk_.createBuffer(
			vbSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingVB, stagingVBMem);

		void* mapped = nullptr;
		vkMapMemory(device, stagingVBMem, 0, vbSize, 0, &mapped);
		std::memcpy(mapped, data.waterVertices.data(), static_cast<size_t>(vbSize));
		vkUnmapMemory(device, stagingVBMem);

		// staging IB
		VkBuffer stagingIB{};
		VkDeviceMemory stagingIBMem{};
		vk_.createBuffer(
			ibSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingIB, stagingIBMem);

		vkMapMemory(device, stagingIBMem, 0, ibSize, 0, &mapped);
		std::memcpy(mapped, data.waterIndices.data(), static_cast<size_t>(ibSize));
		vkUnmapMemory(device, stagingIBMem);

		// device local VB/IB
		vk_.createBuffer(
			vbSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			waterVB_, waterVBMem_);

		vk_.createBuffer(
			ibSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			waterIB_, waterIBMem_);

		// copy staging to device local
		vk_.copyBuffer(stagingVB, waterVB_, vbSize);
		vk_.copyBuffer(stagingIB, waterIB_, ibSize);

		// destroy staging
		vkDestroyBuffer(device, stagingVB, nullptr);
		vkFreeMemory(device, stagingVBMem, nullptr);
		vkDestroyBuffer(device, stagingIB, nullptr);
		vkFreeMemory(device, stagingIBMem, nullptr);
	}
	else
	{
		waterIndexCount_ = 0;
	}

} // end of upload()

void ChunkMeshGPUVk::drawOpaque(const DrawContext& ctx)
{
	auto cmd = static_cast<VkCommandBuffer>(ctx.backendCmd);

	if (!cmd || opaqueIndexCount_ == 0) return;

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &opaqueVB_, &offset);
	vkCmdBindIndexBuffer(cmd, opaqueIB_, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(cmd, opaqueIndexCount_, 1, 0, 0, 0);
} // end of drawOpaque()

void ChunkMeshGPUVk::drawWater(const DrawContext& ctx)
{
	auto cmd = static_cast<VkCommandBuffer>(ctx.backendCmd);

	if (!cmd || waterIndexCount_ == 0) return;

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &waterVB_, &offset);
	vkCmdBindIndexBuffer(cmd, waterIB_, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(cmd, waterIndexCount_, 1, 0, 0, 0);
} // end of drawWater()


//--- PRIVATE ---//
void ChunkMeshGPUVk::destroyBuffers()
{
	VkDevice device = vk_.device();

	if (opaqueVB_)
	{
		vkDestroyBuffer(device, opaqueVB_, nullptr);
		opaqueVB_ = VK_NULL_HANDLE;
	}
	if (opaqueVBMem_)
	{
		vkFreeMemory(device, opaqueVBMem_, nullptr);
		opaqueVBMem_ = VK_NULL_HANDLE;
	}
	if (opaqueIB_)
	{
		vkDestroyBuffer(device, opaqueIB_, nullptr);
		opaqueIB_ = VK_NULL_HANDLE;
	}
	if (opaqueIBMem_)
	{
		vkFreeMemory(device, opaqueIBMem_, nullptr);
		opaqueIBMem_ = VK_NULL_HANDLE;
	}

	if (waterVB_)
	{
		vkDestroyBuffer(device, waterVB_, nullptr);
		waterVB_ = VK_NULL_HANDLE;
	}
	if (waterVBMem_)
	{
		vkFreeMemory(device, waterVBMem_, nullptr);
		waterVBMem_ = VK_NULL_HANDLE;
	}
	if (waterIB_)
	{
		vkDestroyBuffer(device, waterIB_, nullptr);
		waterIB_ = VK_NULL_HANDLE;
	}
	if (waterIBMem_)
	{
		vkFreeMemory(device, waterIBMem_, nullptr);
		waterIBMem_ = VK_NULL_HANDLE;
	}
} // end of destroyBuffers()