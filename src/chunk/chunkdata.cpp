#include "chunkdata.h"

//--- PUBLIC ---//
ChunkData::ChunkData(int cx, int cz)
{
	m_chunkX = cx;
	m_chunkZ = cz;
	blocks_.fill(BlockID::Air);

	setupHeightMap(cx, cz);

	const int seaLevel = 64;
	const int minGround = 40;
	const int maxTerrain = 40;

	for (int x = 0; x < CHUNK_SIZE; ++x)
	{
		for (int z = 0; z < CHUNK_SIZE; ++z)
		{	
			float n = heightMap_.GetValue(x, z);
			float n01 = (n + 1.0f) * 0.5f;

			int height = minGround + static_cast<int>(n01 * maxTerrain);

			if (height < 0)
			{
				height = 0;
			}

			if (height >= CHUNK_SIZE_Y)
			{
				height = CHUNK_SIZE_Y - 1;
			}

			for (int y = 0; y < CHUNK_SIZE_Y; ++y)
			{
				if (y > height)
				{
					if (y <= seaLevel)
					{
						setBlocks(x, y, z, BlockID::Water);
					}
					else
					{
						setBlocks(x, y, z, BlockID::Air);
					}
				}
				else if (y == height)
				{
					if (height < seaLevel + 2)
					{
						setBlocks(x, y, z, BlockID::Sand);
					}
					else
					{
						setBlocks(x, y, z, BlockID::Grass);
					}
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
		y < 0 || y >= CHUNK_SIZE_Y ||
		z < 0 || z >= CHUNK_SIZE)
	{
		return;
	}

	setBlocks(x, y, z, id);
} // end of setBlockID()

const std::array<BlockID, CHUNK_SIZE* CHUNK_SIZE_Y* CHUNK_SIZE>& ChunkData::getBlocks() const
{
	return blocks_;
} // end of getBlocks()

void ChunkData::loadData(std::istream& in)
{
	in.read(
		reinterpret_cast<char*>(blocks_.data()),
		static_cast<std::streamsize>(blocks_.size() * sizeof(BlockID))
		);
} // end of loadData()

//--- PRIVATE ---//
void ChunkData::setBlocks(int x, int y, int z, BlockID id)
{
	blocks_[x + CHUNK_SIZE * (z + CHUNK_SIZE * y)] = id;
} // end of setBlocks()

void ChunkData::setupHeightMap(int cx, int cz)
{
	static module::Perlin terrain;
	terrain.SetSeed(777);
	terrain.SetFrequency(1.0);
	terrain.SetPersistence(0.5);
	terrain.SetLacunarity(2.0);
	terrain.SetOctaveCount(5);

	utils::NoiseMapBuilderPlane heightMapBuilder;
	heightMapBuilder.SetSourceModule(terrain);
	heightMapBuilder.SetDestNoiseMap(heightMap_);

	const double scale = 0.01;

	double worldX0 = (cx * CHUNK_SIZE) * scale;
	double worldX1 = (cx * CHUNK_SIZE + CHUNK_SIZE) * scale;
	double worldZ0 = (cz * CHUNK_SIZE) * scale;
	double worldZ1 = (cz * CHUNK_SIZE + CHUNK_SIZE) * scale;
	
	heightMapBuilder.SetDestSize(CHUNK_SIZE, CHUNK_SIZE);
	heightMapBuilder.SetBounds(worldX0, worldX1, worldZ0, worldZ1);
	heightMapBuilder.Build();
} // end of setupHeightMap()