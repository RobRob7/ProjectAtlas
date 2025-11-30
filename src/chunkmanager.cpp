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