#ifndef CHUNK_MESH_GPU_VK_H
#define CHUNK_MESH_GPU_VK_H

#include "i_chunk_mesh_gpu.h"

#include <vulkan/vulkan.hpp>

#include <cstdint>

class VulkanMain;
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
	vk::UniqueBuffer opaqueVB_{};
	vk::UniqueDeviceMemory opaqueVBMem_{};
	vk::UniqueBuffer opaqueIB_{};
	vk::UniqueDeviceMemory opaqueIBMem_{};
	uint32_t opaqueIndexCount_{ 0 };

	// water
	vk::UniqueBuffer waterVB_{};
	vk::UniqueDeviceMemory waterVBMem_{};
	vk::UniqueBuffer waterIB_{};
	vk::UniqueDeviceMemory waterIBMem_{};
	uint32_t waterIndexCount_{ 0 };
};

#endif
