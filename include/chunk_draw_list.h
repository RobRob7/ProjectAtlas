#ifndef CHUNK_DRAW_LIST_H
#define CHUNK_DRAW_LIST_H

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>

class IChunkMeshGPU;

struct ChunkDrawItem
{
	glm::vec3 chunkOrigin{};
	IChunkMeshGPU* gpu = nullptr;

	uint32_t opaqueIndexCount = 0;
	uint32_t waterIndexCount = 0;

	uint32_t renderedBlockCount = 0;

	bool validOpaque() const noexcept { return gpu && opaqueIndexCount > 0; }
	bool validWater() const noexcept { return gpu && waterIndexCount > 0; }
};

struct ChunkDrawList
{
	std::vector<ChunkDrawItem> items;
	uint32_t frameChunksRendered = 0;
	uint32_t frameBlocksRendered = 0;

	void clear()
	{
		items.clear();
		frameChunksRendered = 0;
		frameBlocksRendered = 0;
	}
};

#endif
