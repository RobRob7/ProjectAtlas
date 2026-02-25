#ifndef CHUNK_ENTRY_H
#define CHUNK_ENTRY_H

#include "chunk_mesh.h"

#include "chunk_mesh_gpu_gl.h"
#include "chunk_mesh_gpu_vk.h"

class IChunkMeshGPU;
class VulkanMain;

#include <memory>

struct ChunkEntry
{
	std::unique_ptr<ChunkMesh> cpu;
	std::unique_ptr<IChunkMeshGPU> gpu;

	ChunkEntry(int chunkX, int chunkZ, VulkanMain* vk)
	{
		cpu = std::make_unique<ChunkMesh>(chunkX, chunkZ);

		if (vk)
		{
			gpu = std::make_unique<ChunkMeshGPUVk>(*vk);
		}
		else
		{
			gpu = std::make_unique<ChunkMeshGPUGL>();
		}

		gpu->upload(cpu->data());
	}

	void rebuildAndUpload()
	{
		cpu->rebuild();
		gpu->upload(cpu->data());
	}
};

#endif
