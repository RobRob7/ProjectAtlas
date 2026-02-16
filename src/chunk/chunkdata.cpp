#include "chunkdata.h"

#include <noise/noise.h>
 
#include <random>
#include <cmath>

//--- PUBLIC ---//
ChunkData::ChunkData(int cx, int cz)
{
	m_chunkX = cx;
	m_chunkZ = cz;
	blocks_.fill(BlockID::Air);

	setupHeightMap(cx, cz);

	std::array<int, CHUNK_SIZE* CHUNK_SIZE> columnHeights;

	for (int x = 0; x < CHUNK_SIZE; ++x)
	{
		for (int z = 0; z < CHUNK_SIZE; ++z)
		{	
			float n = heightMap_.GetValue(x, z);
			float n01 = (n + 1.0f) * 0.5f;

			int height = MIN_GROUND + static_cast<int>(n01 * MAX_TERRAIN);

			if (height < 0)
			{
				height = 0;
			}

			if (height >= CHUNK_SIZE_Y)
			{
				height = CHUNK_SIZE_Y - 1;
			}

			// remember columns ground height
			columnHeights[x + CHUNK_SIZE * z] = height;

			for (int y = 0; y < CHUNK_SIZE_Y; ++y)
			{
				if (y > height)
				{
					if (y <= SEA_LEVEL)
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
					if (height < SEA_LEVEL + 2)
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

	// second pass: place trees after terrain
	for (int x = 0; x < CHUNK_SIZE; ++x)
	{
		for (int z = 0; z < CHUNK_SIZE; ++z)
		{
			int height = columnHeights[x + CHUNK_SIZE * z];
			placeTree(x, height, z);
		} // end for
	} // end for
} // end of constructor

ChunkData::~ChunkData() = default;

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

void ChunkData::placeTree(int x, int groundY, int z)
{
	// should place above sea level
	if (groundY <= SEA_LEVEL + 1)
	{
		return;
	}

	// do not place tree if leaves crossover to next chunk
	const int leafRadius = 4;
	if (x < leafRadius || x >= CHUNK_SIZE - leafRadius ||
		z < leafRadius || z >= CHUNK_SIZE - leafRadius)
	{
		return;
	}

	// ensure enough space for tree
	if (groundY + 7 >= CHUNK_SIZE_Y)
	{
		return;
	}

	// tree should be placed on grass block only
	if (getBlockID(x, groundY, z) != BlockID::Grass)
	{
		return;
	}

	// compute world space coord
	int worldX = m_chunkX * CHUNK_SIZE + x;
	int worldZ = m_chunkZ * CHUNK_SIZE + z;

	// get deterministic random
	uint32_t h = 2166136261u;
	auto mix = [&](int v) {
		h ^= static_cast<uint32_t>(v);
		h *= 16777619u;
		};
	mix(worldX);
	mix(worldZ);

	std::minstd_rand rng(h);

	int percentageOfGrassHaveTree = 7;
	if ((rng() % 100) >= percentageOfGrassHaveTree)
	{
		return;
	}

	// trunk height [4,6] blocks
	int trunkHeight = 4 + (rng() % 3);
	int baseY = groundY + 1;
	int topY = baseY + trunkHeight - 1;

	// place trunk
	for (int ty = baseY; ty <= topY; ++ty)
	{
		if (ty >= 0 && ty < CHUNK_SIZE_Y)
		{
			setBlocks(x, ty, z, BlockID::Tree_Trunk);
		}
	} // end for


	// place leaves
	int canopyBottom = topY - 2;
	int canopyTop = topY + 1;

	for (int y = canopyBottom; y <= canopyTop; ++y)
	{
		if (y < 0 || y >= CHUNK_SIZE_Y)
		{
			continue;
		}

		int dy = y - topY;

		// vertical taper: smaller radius at very top/bottom
		int radius = (dy == -2 || dy == 1) ? 1 : 2;

		for (int dx = -radius; dx <= radius; ++dx)
		{
			int lx = x + dx;
			if (lx < 0 || lx >= CHUNK_SIZE)
				continue;

			for (int dz = -radius; dz <= radius; ++dz)
			{
				int lz = z + dz;
				if (lz < 0 || lz >= CHUNK_SIZE)
					continue;

				// don't overwrite the trunk column on lower layers
				if (dx == 0 && dz == 0 && y <= topY)
				{
					continue;
				}

				// skip corners on the full layers to round it off,
				if (radius == 2 && std::abs(dx) == 2 && std::abs(dz) == 2)
				{
					continue;
				}

				// add tiny randomness for holes in outer leaves.
				if (radius == 2 && (rng() % 5) == 0)
				{
					continue;
				}

				BlockID cur = getBlockID(lx, y, lz);
				if (cur == BlockID::Air || cur == BlockID::Water)
				{
					setBlocks(lx, y, lz, BlockID::Tree_Leaf);
				}
			} // end for
		} // end for
	} // end for
} // end of placeTree()