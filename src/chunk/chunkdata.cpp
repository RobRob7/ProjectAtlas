#include "chunkdata.h"

//--- PUBLIC ---//
ChunkData::ChunkData(int cx, int cz)
{
	m_chunkX = cx;
	m_chunkZ = cz;
	blocks_.fill(BlockID::Air);

	int height = 0;

	for (int x = 0; x < CHUNK_SIZE; ++x)
	{
		for (int z = 0; z < CHUNK_SIZE; ++z)
		{
			for (int y = 0; y < CHUNK_SIZE; ++y)
			{
				height = 8 + int(4 * sin(0.4 * (m_chunkX * CHUNK_SIZE + x + y)) * cos(0.2 * (m_chunkZ * CHUNK_SIZE + z)));
				if (y > height)
				{
					setBlocks(x, y, z, BlockID::Air);
				}
				else if (y == height)
				{
					setBlocks(x, y, z, BlockID::Grass);
				}
				else if (y > height - 3)
				{
					setBlocks(x, y, z, BlockID::Dirt);
				}
				else
				{
					setBlocks(x, y, z, BlockID::Stone);
				}
			} // end for
		} // end for
	} // end for
} // end of constructor

BlockID ChunkData::getBlockID(int x, int y, int z) const
{
	return blocks_[x + CHUNK_SIZE * (z + CHUNK_SIZE * y)];
} // end of getBlockID()

void ChunkData::setBlockID(int x, int y, int z, BlockID id)
{
	if (x < 0 || x >= CHUNK_SIZE ||
		y < 0 || y >= CHUNK_SIZE ||
		z < 0 || z >= CHUNK_SIZE)
	{
		return;
	}

	setBlocks(x, y, z, id);
} // end of setBlockID()

//--- PRIVATE ---//
void ChunkData::setBlocks(int x, int y, int z, BlockID id)
{
	blocks_[x + CHUNK_SIZE * (z + CHUNK_SIZE * y)] = id;
} // end of setBlocks()