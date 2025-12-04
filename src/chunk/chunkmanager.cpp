#include "chunkmanager.h"

//--- PUBLIC ---//
ChunkManager::ChunkManager(int viewRadiusInChunks)
	: viewRadius_(viewRadiusInChunks)
{
} // end of constructor

void ChunkManager::update(const glm::vec3& cameraPos)
{
	// find current chunk camera is in
	int cameraChunkX = static_cast<int>(std::floor(cameraPos.x / CHUNK_SIZE));
	int cameraChunkZ = static_cast<int>(std::floor(cameraPos.z / CHUNK_SIZE));

	//
	for (int dz = -viewRadius_; dz <= viewRadius_; ++dz)
	{
		for (int dx = -viewRadius_; dx <= viewRadius_; ++dx)
		{
			// check chunk coordinate (current)
			ChunkCoord coord{ cameraChunkX + dx, cameraChunkZ + dz };

			// check if we are in "new" chunk
			if (chunks_.find(coord) == chunks_.end())
			{
				// create new chunk
				std::unique_ptr<ChunkMesh> chunk = std::make_unique<ChunkMesh>(coord.x, coord.z);
				chunk->uploadChunkMesh();
				chunks_.emplace(coord, std::move(chunk));
			}
		} // end for
	} // end for

	// identify chunks out of range
	std::vector<ChunkCoord> chunksToRemove;
	for (auto& [coord, chunk] : chunks_)
	{
		int dx = coord.x - cameraChunkX;
		int dz = coord.z - cameraChunkZ;
		if (std::abs(dx) > viewRadius_ || std::abs(dz) > viewRadius_)
		{
			chunksToRemove.push_back(coord);
		}
	} // end for

	// remove out of range chunks
	for (size_t i = 0; i < chunksToRemove.size(); ++i)
	{
		chunks_.erase(chunksToRemove[i]);
	} // end for


} // end of update()

void ChunkManager::render(const glm::mat4& view, const glm::mat4& proj)
{
	for (auto& [coord, chunk] : chunks_)
	{
		chunk->renderChunk(view, proj);
	} // end for
} // end of render()

BlockID ChunkManager::getBlock(int wx, int wy, int wz) const
{
	int chunkX = static_cast<int>(std::floor(wx / static_cast<float>(CHUNK_SIZE)));
	int chunkZ = static_cast<int>(std::floor(wz / static_cast<float>(CHUNK_SIZE)));

	ChunkCoord coord{ chunkX, chunkZ };
	auto it = chunks_.find(coord);
	if (it == chunks_.end())
	{
		return BlockID::Air;
	}

	int localX = wx - chunkX * CHUNK_SIZE;
	int localY = wy;
	int localZ = wz - chunkZ * CHUNK_SIZE;

	if (localX < 0 || localX >= CHUNK_SIZE ||
		localY < 0 || localY >= CHUNK_SIZE ||
		localZ < 0 || localZ >= CHUNK_SIZE)
	{
		return BlockID::Air;
	}

	return it->second->getBlock(localX, localY, localZ);
} // end of getBlock()

void ChunkManager::setBlock(int wx, int wy, int wz, BlockID id)
{
	int chunkX = static_cast<int>(std::floor(wx / static_cast<float>(CHUNK_SIZE)));
	int chunkZ = static_cast<int>(std::floor(wz / static_cast<float>(CHUNK_SIZE)));

	ChunkCoord coord{ chunkX, chunkZ };
	auto it = chunks_.find(coord);
	if (it == chunks_.end())
	{
		return;
	}

	int localX = wx - chunkX * CHUNK_SIZE;
	int localY = wy;
	int localZ = wz - chunkZ * CHUNK_SIZE;

	if (localY < 0 || localY >= CHUNK_SIZE)
	{
		return;
	}

	it->second->setBlock(localX, localY, localZ, id);
	it->second->rebuild();
} // end of setBlock()

BlockHit ChunkManager::raycastBlocks(const glm::vec3& origin, const glm::vec3& dir, float maxDistance, float step) const
{
	BlockHit hit;

	glm::vec3 rayDir = glm::normalize(dir);

	// invalid direction
	if (glm::dot(rayDir, rayDir) < 1e-8f) {
		return hit;
	}

	// start slightly in front of the camera to avoid hitting inside the player cell
	glm::vec3 start = origin + rayDir * 0.001f;

	int x = static_cast<int>(std::floor(start.x));
	int y = static_cast<int>(std::floor(start.y));
	int z = static_cast<int>(std::floor(start.z));

	// step on each axis
	int stepX = (rayDir.x > 0.0f) ? 1 : (rayDir.x < 0.0f ? -1 : 0);
	int stepY = (rayDir.y > 0.0f) ? 1 : (rayDir.y < 0.0f ? -1 : 0);
	int stepZ = (rayDir.z > 0.0f) ? 1 : (rayDir.z < 0.0f ? -1 : 0);

	// distance to first voxel boundary on each axis
	auto initAxis = [&](float originCoord, float dirCoord, int gridCoord, int step) {
		if (step == 0) {
			return std::make_pair(std::numeric_limits<float>::infinity(),
				std::numeric_limits<float>::infinity());
		}

		float nextBoundary = (step > 0) ? (gridCoord + 1.0f) : static_cast<float>(gridCoord);
		float tMax = (nextBoundary - originCoord) / dirCoord;
		float tDelta = std::abs(1.0f / dirCoord);
		return std::make_pair(tMax, tDelta);
		};

	auto [tMaxX, tDeltaX] = initAxis(start.x, rayDir.x, x, stepX);
	auto [tMaxY, tDeltaY] = initAxis(start.y, rayDir.y, y, stepY);
	auto [tMaxZ, tDeltaZ] = initAxis(start.z, rayDir.z, z, stepZ);

	float t = 0.0f;
	glm::ivec3 normal(0);

	while (t <= maxDistance)
	{
		// check current cell
		BlockID id = getBlock(x, y, z);
		if (id != BlockID::Air)
		{
			hit.hit = true;
			hit.block = glm::ivec3(x, y, z);
			hit.normal = normal;
			return hit;
		}

		// step to next voxel
		if (tMaxX < tMaxY && tMaxX < tMaxZ)
		{
			x += stepX;
			t = tMaxX;
			tMaxX += tDeltaX;
			normal = glm::ivec3(-stepX, 0, 0);
		}
		else if (tMaxY < tMaxZ)
		{
			y += stepY;
			t = tMaxY;
			tMaxY += tDeltaY;
			normal = glm::ivec3(0, -stepY, 0);
		}
		else
		{
			z += stepZ;
			t = tMaxZ;
			tMaxZ += tDeltaZ;
			normal = glm::ivec3(0, 0, -stepZ);
		}
	}

	return hit;
} // end of raycastBlocks()