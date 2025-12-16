#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "chunkmesh.h"
#include "shader.h"
#include "texture.h"
#include "save.h"

#include <glm/glm.hpp>

#include <unordered_map>
#include <unordered_set>
#include <queue>
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

	void initShaderTexture();

	void update(const glm::vec3& cameraPos);
	void render(const glm::mat4& view, const glm::mat4& proj);

	BlockID getBlock(int wx, int wy, int wz) const;
	void setBlock(int wx, int wy, int wz, BlockID id);
	void setLastBlockUsed(BlockID block);
	int getViewRadius() const;
	const Shader& getShader() const;

	void placeOrRemoveBlock(bool shouldPlace, const glm::vec3& origin, const glm::vec3& dir, float step = 0.1f);

	void saveWorld();
private:
	Save saveWorld_;
	Shader shader_;
	Texture texture_;

	glm::vec3 lastCameraPos_{};

	int viewRadius_;
	std::unordered_map<ChunkCoord, std::unique_ptr<ChunkMesh>, ChunkCoordHash> chunks_;
	std::queue<ChunkCoord> pendingChunks_;
	std::unordered_set<ChunkCoord, ChunkCoordHash> queuedChunks_;

	// raycast data
	BlockID lastBlockUsed_;
	const float maxDistanceRay_ = 5.0f;
private:
	BlockHit raycastBlocks(const glm::vec3& origin, const glm::vec3& dir, float step = 0.1f) const;
};

#endif
