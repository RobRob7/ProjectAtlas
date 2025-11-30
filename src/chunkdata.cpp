#include "chunkdata.h"

//--- PUBLIC ---//
ChunkData::ChunkData(int cx, int cz)
{
	m_chunkX = cx;
	m_chunkZ = cz;
	blocks_.fill(0);

	for (int x = 0; x < CHUNK_SIZE; ++x)
	{
		for (int z = 0; z < CHUNK_SIZE; ++z)
		{
			for (int y = 0; y < 4; ++y)
			{
				setBlocks(x, y, z, 1);
			} // end for
		} // end for
	} // end for
} // end of constructor

BlockID ChunkData::getBlockID(int x, int y, int z) const
{
	return blocks_[x + CHUNK_SIZE * (z + CHUNK_SIZE * y)];
} // end of getBlockID()

//--- PRIVATE ---//
void ChunkData::setBlocks(int x, int y, int z, BlockID id)
{
	blocks_[x + CHUNK_SIZE * (z + CHUNK_SIZE * y)] = id;
} // end of setBlocks()