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
		return 0;
	}

	int localX = wx - chunkX * CHUNK_SIZE;
	int localY = wy;
	int localZ = wz - chunkZ * CHUNK_SIZE;

	if (localX < 0 || localX >= CHUNK_SIZE ||
		localY < 0 || localY >= CHUNK_SIZE ||
		localZ < 0 || localZ >= CHUNK_SIZE)
	{
		return 0;
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

	// start in front of camera by a bit
	float tStart = 0.3f;

	for (float t = tStart; t <= maxDistance; t += step)
	{
		glm::vec3 pos = origin + rayDir * t;

		int wx = static_cast<int>(std::floor(pos.x));
		int wy = static_cast<int>(std::floor(pos.y));
		int wz = static_cast<int>(std::floor(pos.z));

		BlockID id = getBlock(wx, wy, wz);

		// hitting anything other than air
		if (id != 0)
		{
			hit.hit = true;
			hit.block = { wx, wy, wz };

			glm::vec3 d = rayDir;
			glm::vec3 cell = glm::vec3(wx, wy, wz);
			glm::vec3 frac = pos - cell;

			// distance back to the face we entered from
			float distX = (d.x > 0.0f) ? frac.x : (1.0f - frac.x);
			float distY = (d.y > 0.0f) ? frac.y : (1.0f - frac.y);
			float distZ = (d.z > 0.0f) ? frac.z : (1.0f - frac.z);

			float minDist = distX;
			int axis = 0; // 0=x, 1=y, 2=z

			if (distY < minDist) 
			{ 
				minDist = distY; axis = 1; 
			}
			if (distZ < minDist) 
			{ 
				minDist = distZ; axis = 2; 
			}

			glm::ivec3 normal(0);

			if (axis == 0) 
			{
				normal.x = (d.x > 0.0f) ? -1 : 1;
			}
			else if (axis == 1) 
			{
				normal.y = (d.y > 0.0f) ? -1 : 1;
			}
			else 
			{
				normal.z = (d.z > 0.0f) ? -1 : 1;
			}

			hit.normal = normal;
			break;
		}
	} // end for

	return hit;
} // end of raycastBlocks()