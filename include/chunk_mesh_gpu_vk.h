#ifndef CHUNK_MESH_GPU_VK_H
#define CHUNK_MESH_GPU_VK_H

#include "i_chunk_mesh_gpu.h"
#include "vulkan_main.h"

#include <vulkan/vulkan.h>

#include <cstdint>

struct ChunkMeshData;

class ChunkMeshGPUVk final : public IChunkMeshGPU
{
public:
	explicit ChunkMeshGPUVk(VulkanMain& vk);
	~ChunkMeshGPUVk() override;

	void upload(const ChunkMeshData& data) override;
	void drawOpaque() override;
	void drawWater() override;

private:
	VulkanMain& vk_;

	// opaque
	VkBuffer opaqueVB_{ VK_NULL_HANDLE };
	VkDeviceMemory opaqueVBMem_{ VK_NULL_HANDLE };
	VkBuffer opaqueIB_{ VK_NULL_HANDLE };
	VkDeviceMemory opaqueIBMem_{ VK_NULL_HANDLE };
	uint32_t opaqueIndexCount_{ 0 };

	// water
	VkBuffer waterVB_{ VK_NULL_HANDLE };
	VkDeviceMemory waterVBMem_{ VK_NULL_HANDLE };
	VkBuffer waterIB_{ VK_NULL_HANDLE };
	VkDeviceMemory waterIBMem_{ VK_NULL_HANDLE };
	uint32_t waterIndexCount_{ 0 };
private:
	void destroyBuffers();
};

#endif
