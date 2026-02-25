#ifndef CHUNK_DATA_H
#define CHUNK_DATA_H

#include "constants.h"

#include <noise/noiseutils.h>

#include <cstdint>
#include <array>
#include <istream>

using namespace World;

class ChunkData
{
public:
	int m_chunkX;
	int m_chunkZ;
	bool m_dirty = false;
public:
	ChunkData(int cx, int cz);
	~ChunkData();

	BlockID getBlockID(int x, int y, int z) const;
	void setBlockID(int x, int y, int z, BlockID id);

	const std::array<BlockID, CHUNK_SIZE * CHUNK_SIZE_Y* CHUNK_SIZE>& getBlocks() const;

	void loadData(std::istream& in);

private:
	std::array<BlockID, CHUNK_SIZE * CHUNK_SIZE_Y* CHUNK_SIZE> blocks_;
	// libnoise
	utils::NoiseMap heightMap_;
private:
	void setBlocks(int x, int y, int z, BlockID id);
	void setupHeightMap(int cx, int cz);
	void placeTree(int x, int groundY, int z);
};

#endif
