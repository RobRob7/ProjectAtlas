#ifndef CHUNKDATA_H
#define CHUNKDATA_H

#include <noise/noiseutils.h>

#include <cstdint>
#include <array>
#include <istream>

// blocks
enum class BlockID : uint8_t
{
	Air,
	Dirt,
	Grass,
	Stone,
	Tree_Leaf,
	Tree_Trunk,
	Glow_Block,
	Sand,
	Water
};

inline constexpr int CHUNK_SIZE = 15;
inline constexpr int CHUNK_SIZE_Y = 256;

inline constexpr int SEA_LEVEL = 64;
inline constexpr int MIN_GROUND = 40;
inline constexpr int MAX_TERRAIN = 40;

class ChunkData
{
public:
	int m_chunkX;
	int m_chunkZ;
	bool m_dirty = false;
public:
	ChunkData(int cx, int cz);

	BlockID getBlockID(int x, int y, int z) const;
	void setBlockID(int x, int y, int z, BlockID id);

	const std::array<BlockID, CHUNK_SIZE* CHUNK_SIZE_Y* CHUNK_SIZE>& getBlocks() const;

	void loadData(std::istream& in);

private:
	std::array<BlockID, CHUNK_SIZE* CHUNK_SIZE_Y* CHUNK_SIZE> blocks_;
	// libnoise
	utils::NoiseMap heightMap_;
private:
	void setBlocks(int x, int y, int z, BlockID id);
	void setupHeightMap(int cx, int cz);
	void placeTree(int x, int groundY, int z);
};

#endif
