#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "chunkmesh.h"
#include "shader.h"
#include "texture.h"
#include "save.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <memory>
#include <optional>
#include <cstdint>

struct BlockHit
{
	bool hit = false;
	glm::ivec3 block{};
	glm::ivec3 normal{};
};

class ChunkManager
{
public:
	ChunkManager(int viewRadiusInChunks = 2);

	void init();

	void update(const glm::vec3& cameraPos);
	void renderOpaque(const glm::mat4& view, const glm::mat4& proj);
	void renderOpaque(Shader& shader, const glm::mat4& view, const glm::mat4& proj);
	void renderWater(const glm::mat4& view, const glm::mat4& proj);

	BlockID getBlock(int wx, int wy, int wz) const;
	void setBlock(int wx, int wy, int wz, BlockID id);
	void setLastBlockUsed(BlockID block);
	int getViewRadius() const;
	void setViewRadius(int r);

	std::optional<Shader>& getOpaqueShader();
	std::optional<Shader>& getWaterShader();

	const glm::vec3& getLastCameraPos() const;

	float getAmbientStrength() const;
	void setAmbientStrength(float strength);

	void placeOrRemoveBlock(bool shouldPlace, const glm::vec3& origin, const glm::vec3& dir, float step = 0.1f);

	void saveWorld();

	uint32_t getFrameChunksRendered() const;
	uint32_t getFrameBlocksRendered() const;

	bool statusFrustumCulling() const;
	void enableFrustumCulling(bool enable);

private:
	float ambientStrength_ = 0.5f;
	Save saveWorld_;

	// opaque + water shader
	std::optional<Shader> opaqueShader_;
	std::optional<Shader> waterShader_;

	// texture atlas
	std::optional<Texture> atlas_;

	// frustum culling toggle
	bool enableFrustumCulling_ = true;

	// count
	uint32_t frameChunksRendered_ = 0;
	uint32_t frameBlocksRendered_ = 0;

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
