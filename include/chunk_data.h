#ifndef CHUNK_DATA_H
#define CHUNK_DATA_H

#include "constants.h"

#include <noise/noiseutils.h>

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

	BlockID getBlockID(int x, int y, int z) const
	{
		return blocks_[x + CHUNK_SIZE * (z + CHUNK_SIZE * y)];
	} // end of getBlockID()

	void setBlockID(int x, int y, int z, BlockID id)
	{
		if (x < 0 || x >= CHUNK_SIZE ||
			y < 0 || y >= CHUNK_SIZE_Y ||
			z < 0 || z >= CHUNK_SIZE)
		{
			return;
		}

		setBlocks(x, y, z, id);
	} // end of setBlockID()

	void loadData(std::istream& in)
	{
		in.read(
			reinterpret_cast<char*>(blocks_.data()),
			static_cast<std::streamsize>(blocks_.size() * sizeof(BlockID))
		);
	} // end of loadData()

	const std::array<BlockID, CHUNK_SIZE* CHUNK_SIZE_Y* CHUNK_SIZE>& getBlocks() const
	{
		return blocks_;
	} // end of getBlocks()

private:
	std::array<BlockID, CHUNK_SIZE * CHUNK_SIZE_Y * CHUNK_SIZE> blocks_;
	utils::NoiseMap heightMap_;
	module::Perlin caveNoise_;
private:
	void setBlocks(int x, int y, int z, BlockID id);
	void setupHeightMap(int cx, int cz);
	void setupCaveNoise();
	void carveCave(int x, int y, int z);
	void placeTree(int x, int groundY, int z);
};

#endif
