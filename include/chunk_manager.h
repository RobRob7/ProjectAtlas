#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include "constants.h"

#include "save.h"
#include "chunk_draw_list.h"
#include "chunk_mesh.h"

#include <glm/glm.hpp>

#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <memory>
#include <cstdint>

struct ChunkEntry;
class VulkanMain;

using namespace World;

struct BlockHit
{
	bool hit = false;
	glm::ivec3 block{};
	glm::ivec3 normal{};
};

class ChunkManager
{
public:
	static constexpr int MIN_RADIUS = 5;
	static constexpr int MAX_RADIUS = 100;
	static constexpr float MIN_AMBSTR = 0.05f;
	static constexpr float MAX_AMBSTR = 0.5f;
public:
	ChunkManager(int viewRadiusInChunks = 15);
	~ChunkManager();

	void init(VulkanMain* vk);

	void update(const glm::vec3& cameraPos);

	void buildOpaqueDrawList(const glm::mat4& view, const glm::mat4& proj, ChunkDrawList& out);
	void buildWaterDrawList(const glm::mat4& view, const glm::mat4& proj, ChunkDrawList& out);

	BlockID getBlock(int wx, int wy, int wz) const;
	void setBlock(int wx, int wy, int wz, BlockID id);
	void setLastBlockUsed(BlockID block);
	int getViewRadius() const;
	void setViewRadius(int r);

	const glm::vec3& getLastCameraPos() const;

	float getAmbientStrength() const;
	void setAmbientStrength(float strength);

	void placeOrRemoveBlock(bool shouldPlace, const glm::vec3& origin, const glm::vec3& dir);

	void saveWorld();

	uint32_t getFrameChunksRendered() const;
	uint32_t getFrameBlocksRendered() const;

	bool statusFrustumCulling() const;
	void enableFrustumCulling(bool enable);

private:
	float ambientStrength_{ 0.5f };
	Save saveWorld_;

	// frustum culling toggle
	bool enableFrustumCulling_ = true;

	// count
	uint32_t frameChunksRendered_{ 0 };
	uint32_t frameBlocksRendered_{ 0 };

	glm::vec3 lastCameraPos_{};

	int viewRadius_;
	std::unordered_map<ChunkCoord, std::unique_ptr<ChunkEntry>, ChunkCoordHash> chunks_;
	std::queue<ChunkCoord> pendingChunks_;
	std::unordered_set<ChunkCoord, ChunkCoordHash> queuedChunks_;

	VulkanMain* vk_ = nullptr;

	// raycast data
	BlockID lastBlockUsed_;
	static constexpr float maxDistanceRay_ = 5.0f;
private:
	BlockHit raycastBlocks(const glm::vec3& origin, const glm::vec3& dir) const;
};

#endif
