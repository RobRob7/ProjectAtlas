#ifndef CHUNKDATA_H
#define CHUNKDATA_H

#include <glm/glm.hpp>

#include <cstdint>
#include <array>

// 0 = air
// 1 = dirt
// 2 = dirt (grass)
// 3 = stone
using BlockID = uint8_t;

inline constexpr int CHUNK_SIZE = 32;

class ChunkData
{
public:
	int m_chunkX;
	int m_chunkZ;
public:
	ChunkData(int cx, int cz);

	BlockID getBlockID(int x, int y, int z) const;

private:
	std::array<BlockID, CHUNK_SIZE* CHUNK_SIZE* CHUNK_SIZE> blocks_;
private:
	void setBlocks(int x, int y, int z, BlockID id);
};

#endif
