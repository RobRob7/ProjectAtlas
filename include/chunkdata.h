#ifndef CHUNKDATA_H
#define CHUNKDATA_H

#include <glm/glm.hpp>

#include <cstdint>
#include <array>

// blocks
enum class BlockID : uint8_t
{
	Air,
	Dirt,
	Grass,
	Stone,
	Tree_Leaf,
	Glow_Block
};

inline constexpr int CHUNK_SIZE = 16;
inline constexpr int CHUNK_SIZE_Y = 256;

class ChunkData
{
public:
	int m_chunkX;
	int m_chunkZ;
public:
	ChunkData(int cx, int cz);

	BlockID getBlockID(int x, int y, int z) const;
	void setBlockID(int x, int y, int z, BlockID id);

private:
	std::array<BlockID, CHUNK_SIZE* CHUNK_SIZE_Y* CHUNK_SIZE> blocks_;
private:
	void setBlocks(int x, int y, int z, BlockID id);
};

#endif
