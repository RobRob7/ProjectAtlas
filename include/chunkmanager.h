#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "chunkmesh.h"

#include <glm/glm.hpp>

#include <unordered_map>
#include <vector>
#include <memory>
#include <cmath>

struct BlockHit
{
	bool hit = false;
	glm::ivec3 block;
	glm::ivec3 normal;
};

class ChunkManager
{
public:
	ChunkManager(int viewRadiusInChunks = 2);

	void update(const glm::vec3& cameraPos);
	void render(const glm::mat4& view, const glm::mat4& proj);

	BlockID getBlock(int wx, int wy, int wz) const;
	void setBlock(int wx, int wy, int wz, BlockID id);
	
	BlockHit raycastBlocks(const glm::vec3& origin, const glm::vec3& dir, float maxDistance, float step = 0.1f) const;

private:
	int viewRadius_;
	std::unordered_map<ChunkCoord, std::unique_ptr<ChunkMesh>, ChunkCoordHash> chunks_;
};

#endif
