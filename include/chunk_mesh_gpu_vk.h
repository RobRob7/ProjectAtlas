#ifndef CHUNK_MESH_GPU_VK_H
#define CHUNK_MESH_GPU_VK_H

#include "i_chunk_mesh_gpu.h"

#include "buffer_vk.h"

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
	void drawOpaque(vk::CommandBuffer cmd) override;
	void drawWater(vk::CommandBuffer cmd) override;

private:
	VulkanMain& vk_;

	// opaque
	BufferVk opaqueVB_;
	BufferVk opaqueIB_;
	uint32_t opaqueIndexCount_{ 0 };

	// water
	BufferVk waterVB_;
	BufferVk waterIB_;
	uint32_t waterIndexCount_{ 0 };
};

#endif
