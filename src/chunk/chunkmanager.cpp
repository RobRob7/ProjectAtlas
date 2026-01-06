#include "chunkmanager.h"

//--- HELPER ---//
struct Plane
{
	glm::vec3 n; // normal
	float d;     // plane: dot(n, x) + d >= 0 is inside
};

struct Frustum
{
	Plane p[6]; // L, R, B, T, N, F
};

static Plane NormalizePlane(const Plane& pl)
{
	float len = glm::length(pl.n);
	if (len <= 1e-8f) return pl;
	return { pl.n / len, pl.d / len };
} // end of NormalizePlane()

static Frustum ExtractFrustumPlanes(const glm::mat4& VP)
{
	// GLM is column-major; to get row r: (VP[0][r], VP[1][r], VP[2][r], VP[3][r])
	auto row = [&](int r) {
		return glm::vec4(VP[0][r], VP[1][r], VP[2][r], VP[3][r]);
		};

	glm::vec4 r0 = row(0);
	glm::vec4 r1 = row(1);
	glm::vec4 r2 = row(2);
	glm::vec4 r3 = row(3);

	auto makePlane = [&](const glm::vec4& v) {
		return NormalizePlane(Plane{ glm::vec3(v), v.w });
		};

	Frustum f;
	f.p[0] = makePlane(r3 + r0); // Left
	f.p[1] = makePlane(r3 - r0); // Right
	f.p[2] = makePlane(r3 + r1); // Bottom
	f.p[3] = makePlane(r3 - r1); // Top
	f.p[4] = makePlane(r3 + r2); // Near
	f.p[5] = makePlane(r3 - r2); // Far
	return f;
} // end of ExtractFrustumPlanes()

struct AABB
{
	glm::vec3 min;
	glm::vec3 max;
};

static glm::vec3 PositiveVertex(const AABB& b, const glm::vec3& n)
{
	return glm::vec3(
		(n.x >= 0.0f) ? b.max.x : b.min.x,
		(n.y >= 0.0f) ? b.max.y : b.min.y,
		(n.z >= 0.0f) ? b.max.z : b.min.z
	);
} // end of PositiveVertex()

static bool IntersectsFrustum(const AABB& box, const Frustum& f)
{
	for (int i = 0; i < 6; ++i)
	{
		const Plane& p = f.p[i];
		glm::vec3 v = PositiveVertex(box, p.n);

		// if the “most inside” corner is still outside, whole AABB is outside
		if (glm::dot(p.n, v) + p.d < 0.0f)
			return false;
	} // end for
	return true;
} // end of IntersectsFrustum()

static AABB ChunkWorldAABB(int chunkX, int chunkZ)
{
	glm::vec3 base = glm::vec3(chunkX * CHUNK_SIZE, 0.0f, chunkZ * CHUNK_SIZE);
	glm::vec3 size = glm::vec3(CHUNK_SIZE, CHUNK_SIZE_Y, CHUNK_SIZE);

	return { base, base + size };
} // end of ChunkWorldAABB()


//--- PUBLIC ---//
ChunkManager::ChunkManager(int viewRadiusInChunks)
	: viewRadius_(viewRadiusInChunks), lastBlockUsed_(BlockID::Dirt)
{
} // end of constructor

void ChunkManager::init()
{
	shader_.emplace("chunk/chunk.vert", "chunk/chunk.frag");
	texture_.emplace("blocks.png", true);
} // end of initShaderTexture()

void ChunkManager::update(const glm::vec3& cameraPos)
{
	lastCameraPos_ = cameraPos;

	// find current chunk camera is in
	int cameraChunkX = static_cast<int>(std::floor(cameraPos.x / CHUNK_SIZE));
	int cameraChunkZ = static_cast<int>(std::floor(cameraPos.z / CHUNK_SIZE));

	for (int dz = -viewRadius_; dz <= viewRadius_; ++dz)
	{
		for (int dx = -viewRadius_; dx <= viewRadius_; ++dx)
		{
			// check chunk coordinate (current)
			ChunkCoord coord{ cameraChunkX + dx, cameraChunkZ + dz };

			// check if we are in "new" chunk
			if (chunks_.find(coord) == chunks_.end())
			{
				if (queuedChunks_.insert(coord).second)
				{
					// add to pending chunks
					pendingChunks_.push(coord);
				}
			}
		} // end for
	} // end for

	// identify chunks out of range
	for (auto it = chunks_.begin(); it != chunks_.end();)
	{
		int dx = it->first.x - cameraChunkX;
		int dz = it->first.z - cameraChunkZ;
		if (std::abs(dx) > viewRadius_ || std::abs(dz) > viewRadius_)
		{
			// save to file if chunk is dirty
			if (it->second->getChunk().m_dirty)
			{
				saveWorld_.saveChunkToFile(it->second->getChunk(), "HelloWorld");
				it->second->getChunk().m_dirty = false;
			}

			it = chunks_.erase(it);
		}
		else
		{
			++it;
		}
	} // end for

	const int maxNewChunksPerFrame = 2;
	int built = 0;
	// load up to maxNewChunksPerFrame
	while (!pendingChunks_.empty() && built < maxNewChunksPerFrame)
	{
		ChunkCoord coord = pendingChunks_.front();
		pendingChunks_.pop();
		queuedChunks_.erase(coord);

		// chunk already loaded
		if (chunks_.find(coord) != chunks_.end())
		{
			continue;
		}

		// create chunk and upload
		std::unique_ptr<ChunkMesh> chunk = std::make_unique<ChunkMesh>(coord.x, coord.z, *shader_, *texture_);
		std::unique_ptr<ChunkData> loaded = saveWorld_.loadChunkFromFile(coord.x, coord.z, "HelloWorld");
		if (loaded)
		{
			chunk->getChunk() = *loaded;
			chunk->rebuild();
		}

		chunks_.emplace(coord, std::move(chunk));
		++built;
	} // end while

} // end of update()

void ChunkManager::render(const glm::mat4& view, const glm::mat4& proj)
{
	shader_->use();
	glBindTextureUnit(0, texture_->m_ID);
	shader_->setMat4("u_view", view);
	shader_->setMat4("u_proj", proj);

	//
	Frustum fr = ExtractFrustumPlanes(proj * view);
	frameBlocksRendered_ = 0;
	frameChunksRendered_ = 0;
	for (auto& [coord, chunk] : chunks_)
	{
		//
		AABB box = ChunkWorldAABB(chunk->getChunk().m_chunkX, chunk->getChunk().m_chunkZ);
		if (enableFrustrumCulling_ && !IntersectsFrustum(box, fr))
		{
			continue;
		}

		// update count
		++frameChunksRendered_;
		frameBlocksRendered_ += chunk->getRenderedBlockCount();

		chunk->renderChunk();
	} // end for
} // end of render()

void ChunkManager::render(Shader& shader, const glm::mat4& view, const glm::mat4& proj)
{
	shader.use();
	shader.setMat4("u_view", view);
	shader.setMat4("u_proj", proj);

	//
	Frustum fr = ExtractFrustumPlanes(proj * view);
	frameBlocksRendered_ = 0;
	frameChunksRendered_ = 0;
	for (auto& [coord, chunk] : chunks_)
	{
		//
		AABB box = ChunkWorldAABB(chunk->getChunk().m_chunkX, chunk->getChunk().m_chunkZ);
		if (enableFrustrumCulling_ && !IntersectsFrustum(box, fr))
		{
			continue;
		}

		// update count
		++frameChunksRendered_;
		frameBlocksRendered_ += chunk->getRenderedBlockCount();

		chunk->renderChunk(shader);
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
		localY < 0 || localY >= CHUNK_SIZE_Y ||
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

	if (localY < 0 || localY >= CHUNK_SIZE_Y)
	{
		return;
	}

	it->second->setBlock(localX, localY, localZ, id);
	it->second->rebuild();

	// mark chunk as modified
	it->second->getChunk().m_dirty = true;
} // end of setBlock()

void ChunkManager::setLastBlockUsed(BlockID block)
{
	lastBlockUsed_ = block;
} // end of setLastBlockUsed()

int ChunkManager::getViewRadius() const
{
	return viewRadius_;
} // end of getViewRadius()

void ChunkManager::setViewRadius(int r)
{
	viewRadius_ = r;
} // end of setViewRadius()

const std::optional<Shader>& ChunkManager::getShader() const
{
	return shader_;
} // end of getShader()

const glm::vec3& ChunkManager::getLastCameraPos() const
{
	return lastCameraPos_;
} // end of getLastCameraPos()

float ChunkManager::getAmbientStrength() const
{
	return ambientStrength_;
} // end of getAmbientStrength()

void ChunkManager::setAmbientStrength(float strength)
{
	ambientStrength_ = strength;
} // end of setAmbientStrength()

void ChunkManager::placeOrRemoveBlock(bool shouldPlace, const glm::vec3& origin, const glm::vec3& dir, float step)
{
	BlockHit hit = raycastBlocks(origin, dir, maxDistanceRay_);
	// place op
	if (shouldPlace)
	{
		if (hit.hit)
		{
			glm::ivec3 placePos = hit.block + hit.normal;
			setBlock(placePos.x, placePos.y, placePos.z, lastBlockUsed_);
		}
	}
	// destroy op
	else
	{
		if (hit.hit)
		{
			// DISALLOW water deletion by player
			if (getBlock(hit.block.x, hit.block.y, hit.block.z) == BlockID::Water) return;

			setBlock(hit.block.x, hit.block.y, hit.block.z, BlockID::Air);
		}
	}
} // end of placeOrRemoveBlock()

void ChunkManager::saveWorld()
{
	// save all loaded chunks that have been modified
	for (auto& [coord, chunkMesh] : chunks_)
	{
		ChunkData& chunk = chunkMesh->getChunk();

		if (!chunk.m_dirty)
		{
			continue;
		}
		saveWorld_.saveChunkToFile(chunk, "HelloWorld");
		chunk.m_dirty = false;

	} // end for
} // end of saveWorld()

uint32_t ChunkManager::getFrameChunksRendered() const
{
	return frameChunksRendered_;
} // end of getFrameChunksRendered()

uint32_t ChunkManager::getFrameBlocksRendered() const
{
	return frameBlocksRendered_;
} // end of getFrameBlocksRendered()

bool ChunkManager::statusFrustrumCulling() const
{
	return enableFrustrumCulling_;
} // end of statusFrustrumCulling()

void ChunkManager::enableFrustrumCulling(bool enable)
{
	enableFrustrumCulling_ = enable;
} // end of enableFrustrumCulling()


//--- PRIVATE ---//
BlockHit ChunkManager::raycastBlocks(const glm::vec3& origin, const glm::vec3& dir, float step) const
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

	while (t <= maxDistanceRay_)
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