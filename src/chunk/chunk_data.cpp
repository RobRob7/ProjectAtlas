#include "chunk_data.h"

#include "constants.h"

#include <noise/noise.h>
 
#include <random>
#include <array>

//--- PUBLIC ---//
ChunkData::ChunkData(int cx, int cz)
{
	m_chunkX = cx;
	m_chunkZ = cz;
	blocks_.fill(BlockID::Air);

	setupHeightMap(cx, cz);
	setupCaveNoise();

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
					if (y <= World::SEA_LEVEL)
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
					if (height < World::SEA_LEVEL + 2)
					{
						setBlocks(x, y, z, BlockID::Sand);
					}
					else
					{
						setBlocks(x, y, z, BlockID::SnowGrass);
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

	// carve caves after terrain, before trees
	for (int x = 0; x < CHUNK_SIZE; ++x)
	{
		for (int z = 0; z < CHUNK_SIZE; ++z)
		{
			int height = columnHeights[x + CHUNK_SIZE * z];

			for (int y = 1; y < height - 4; ++y)
			{
				carveCave(x, y, z);

				BlockID cur = getBlockID(x, y, z);

				// only place ore in remaining stone
				if (cur == BlockID::Stone)
				{
					int worldX = m_chunkX * CHUNK_SIZE + x;
					int worldZ = m_chunkZ * CHUNK_SIZE + z;

					uint32_t h = 2166136261u;

					auto mix = [&](int v)
						{
							h ^= static_cast<uint32_t>(v);
							h *= 16777619u;
						};

					mix(worldX);
					mix(y);
					mix(worldZ);

					std::minstd_rand rng(h);

					if (y < 30 && (rng() % 100) == 0)
					{
						setBlocks(x, y, z, BlockID::IronOre);
					}

					if (y < 20 && (rng() % 1000) == 0)
					{
						setBlocks(x, y, z, BlockID::DiamondOre);
					}

					if (y < 10 && (rng() % 50000) == 0)
					{
						setBlocks(x, y, z, BlockID::GoldOre);
					}
				}
			} // end for
		} // end for
	} // end for

	// place trees after terrain
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

void ChunkData::setupCaveNoise()
{
	caveNoise_.SetSeed(777);
	caveNoise_.SetFrequency(0.4);
	caveNoise_.SetPersistence(0.5);
	caveNoise_.SetLacunarity(2.0);
	caveNoise_.SetOctaveCount(3);
} // end of setupCaveNoise()

void ChunkData::carveCave(int x, int y, int z)
{
	int worldX = m_chunkX * CHUNK_SIZE + x;
	int worldZ = m_chunkZ * CHUNK_SIZE + z;

	double scale = 0.06;

	double n = caveNoise_.GetValue(
		worldX * scale,
		y * scale,
		worldZ * scale
	);

	if (n > 0.65)
	{
		BlockID cur = getBlockID(x, y, z);

		if (cur == BlockID::Stone || cur == BlockID::Dirt)
		{
			setBlocks(x, y, z, BlockID::Air);
		}
	}
} // end of carveCave()

void ChunkData::placeTree(int x, int groundY, int z)
{
	// should place above sea level
	if (groundY <= World::SEA_LEVEL + 1)
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
	if ((getBlockID(x, groundY, z) != BlockID::Grass) &&
		(getBlockID(x, groundY, z) != BlockID::SnowGrass))
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

	int percentageOfGrassHaveTree = 35;
	if ((rng() % 100) >= percentageOfGrassHaveTree)
	{
		return;
	}

	int trunkHeight = 4 + (rng() % 10);
	int baseY = groundY + 1;
	int topY = baseY + trunkHeight - 5;

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

				// skip corners on the full layers to round it off
				if (radius == 2 && std::abs(dx) == 2 && std::abs(dz) == 2)
				{
					continue;
				}

				// add tiny randomness for holes in outer leaves
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