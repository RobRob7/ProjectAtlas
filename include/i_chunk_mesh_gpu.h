#ifndef I_CHUNK_MESH_GPU_H
#define I_CHUNK_MESH_GPU_H

#include "vulkan/vulkan.h"

struct ChunkMeshData;

struct DrawContext
{
	void* backendCmd = nullptr;
};

class IChunkMeshGPU
{
public:
	virtual ~IChunkMeshGPU() = default;

	virtual void upload(const ChunkMeshData& data) = 0;
	virtual void drawOpaque(const DrawContext& ctx) = 0;
	virtual void drawWater(const DrawContext& ctx) = 0;
};

#endif
