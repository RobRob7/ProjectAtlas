#include "chunkmesh.h"

#include <glad/glad.h>

#include <cstddef>

//--- HELPER ---//
// world opaque vertices
// LAYOUT (32u bits)
// 0  - 1   : UV corner index
// 2  - 6   : tileY
// 7  - 11  : tileX
// 12 - 14  : normal index
// 15 - 18  : x pos
// 19 - 27  : y pos
// 28 - 31  : z pos
static inline uint32_t PackVertexU32(
	uint32_t uvCorner, uint32_t tileX, uint32_t tileY,
	uint32_t normalIdx, uint32_t x, uint32_t y, uint32_t z)
{
	uint32_t packed{};
	// uv corner index
	packed |= (uvCorner & 3u) << 0;

	// UV tileY
	packed |= (tileY & 31u) << 2;
	// UV tileX
	packed |= (tileX & 31u) << 7;

	// normal index
	packed |= (normalIdx & 7u) << 12;

	// pos
	packed |= (x & 15u) << 15;
	packed |= (y & 511u) << 19;
	packed |= (z & 15u) << 28;

	return packed;
} // end of PackVertexU32()


//--- PUBLIC ---//
ChunkMesh::ChunkMesh(int chunkX, int chunkY)
	: chunkData_(chunkX, chunkY)
{
	// OPAQUE
	// create VAO + buffers
	glCreateVertexArrays(1, &opaqueVao_);
	glCreateBuffers(1, &opaqueVbo_);
	glCreateBuffers(1, &opaqueEbo_);

	// attach buffers to vao
	glVertexArrayVertexBuffer(opaqueVao_, 0, opaqueVbo_, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(opaqueVao_, opaqueEbo_);

	// combined data packed in int
	glEnableVertexArrayAttrib(opaqueVao_, 0);
	glVertexArrayAttribIFormat(opaqueVao_, 0, 1, GL_UNSIGNED_INT, offsetof(Vertex, sample));
	glVertexArrayAttribBinding(opaqueVao_, 0, 0);


	// WATER
	// create VAO + buffers
	glCreateVertexArrays(1, &waterVao_);
	glCreateBuffers(1, &waterVbo_);
	glCreateBuffers(1, &waterEbo_);

	// attach buffers to vao
	glVertexArrayVertexBuffer(waterVao_, 0, waterVbo_, 0, sizeof(VertexWater));
	glVertexArrayElementBuffer(waterVao_, waterEbo_);

	// position
	glEnableVertexArrayAttrib(waterVao_, 0);
	glVertexArrayAttribFormat(waterVao_, 0, 3, GL_FLOAT, GL_FALSE, offsetof(VertexWater, pos));
	glVertexArrayAttribBinding(waterVao_, 0, 0);

	buildChunkMesh();
	uploadChunkMesh();
} // end of other constructor

ChunkMesh::~ChunkMesh()
{
	if (opaqueVao_)
	{
		glDeleteVertexArrays(1, &opaqueVao_);
		opaqueVao_ = 0;
	}
	if (opaqueVbo_)
	{
		glDeleteBuffers(1, &opaqueVbo_);
		opaqueVbo_ = 0;
	}
	if (opaqueEbo_)
	{
		glDeleteBuffers(1, &opaqueEbo_);
		opaqueEbo_ = 0;
	}

	if (waterVao_)
	{
		glDeleteVertexArrays(1, &waterVao_);
		waterVao_ = 0;
	}
	if (waterVbo_)
	{
		glDeleteBuffers(1, &waterVbo_);
		waterVbo_ = 0;
	}
	if (waterEbo_)
	{
		glDeleteBuffers(1, &waterEbo_);
		waterEbo_ = 0;
	}
} // end of destructor

void ChunkMesh::uploadChunkMesh()
{
	// OPAQUE reupload into vbo
	glNamedBufferData(
		opaqueVbo_,
		opaqueVertices_.size() * sizeof(Vertex),
		opaqueVertices_.empty() ? nullptr : opaqueVertices_.data(),
		GL_STATIC_DRAW
	);

	// reupload into ebo
	glNamedBufferData(
		opaqueEbo_,
		opaqueIndices_.size() * sizeof(uint32_t),
		opaqueIndices_.empty() ? nullptr : opaqueIndices_.data(),
		GL_STATIC_DRAW
	);

	opaqueIndexCount_ = static_cast<int32_t>(opaqueIndices_.size());


	// WATER reupload into vbo
	glNamedBufferData(
		waterVbo_,
		waterVertices_.size() * sizeof(VertexWater),
		waterVertices_.empty() ? nullptr : waterVertices_.data(),
		GL_STATIC_DRAW
	);

	// reupload into ebo
	glNamedBufferData(
		waterEbo_,
		waterIndices_.size() * sizeof(uint32_t),
		waterIndices_.empty() ? nullptr : waterIndices_.data(),
		GL_STATIC_DRAW
	);

	waterIndexCount_ = static_cast<int32_t>(waterIndices_.size());
} // end of uploadChunkMesh()

void ChunkMesh::renderChunkOpaque()
{
	glBindVertexArray(opaqueVao_);
	glDrawElements(GL_TRIANGLES, opaqueIndexCount_, GL_UNSIGNED_INT, nullptr);
} // end of render()

void ChunkMesh::renderChunkWater()
{
	if (waterIndexCount_ <= 0) return;

	glBindVertexArray(waterVao_);
	glDrawElements(GL_TRIANGLES, waterIndexCount_, GL_UNSIGNED_INT, nullptr);
} // end of renderWater()

void ChunkMesh::setBlock(int x, int y, int z, BlockID id)
{
	chunkData_.setBlockID(x, y, z, id);
} // end of setBlock()

BlockID ChunkMesh::getBlock(int x, int y, int z) const
{
	return chunkData_.getBlockID(x, y, z);
} // end of getBlock()

ChunkData& ChunkMesh::getChunk()
{
	return chunkData_;
} // end of getChunk()

void ChunkMesh::rebuild()
{
	buildChunkMesh();
	uploadChunkMesh();
} // end of rebuild()

uint32_t ChunkMesh::getRenderedBlockCount() const
{
	return renderedBlockCount_;
} // end of getRenderedBlockCount()


//--- PRIVATE ---//
void ChunkMesh::buildChunkMesh()
{
	renderedBlockCount_ = 0;

	opaqueVertices_.clear();
	opaqueIndices_.clear();

	// check if block is opaque
	auto isOpaque = [&](BlockID id) {
		return id != BlockID::Air && id != BlockID::Water;
		};

	// all attr must match for cells to merge
	struct MaskCell {
		bool valid = false;
		BlockID id{};
		FaceDir dir{};
		int tileX = 0;
		int tileY = 0;
	};

	// create quad data
	auto emitQuad = [&](glm::ivec3 p0, glm::ivec3 p1, glm::ivec3 p2, glm::ivec3 p3,
		FaceDir dir, int tileX, int tileY)
		{
			// base vertex index for quad
			uint32_t start = static_cast<uint32_t>(opaqueVertices_.size());

			// vert pos order
			glm::ivec3 corners[4] = { p0, p1, p2, p3 };

			// pack vertex data
			for (int c = 0; c < 4; ++c) {
				Vertex v{};
				v.sample = PackVertexU32(
					static_cast<uint32_t>(c),
					static_cast<uint32_t>(tileX), static_cast<uint32_t>(tileY),
					static_cast<uint32_t>(dir),
					static_cast<uint32_t>(corners[c].x),
					static_cast<uint32_t>(corners[c].y),
					static_cast<uint32_t>(corners[c].z)
				);
				opaqueVertices_.push_back(v);
			}

			opaqueIndices_.push_back(start + 0);
			opaqueIndices_.push_back(start + 1);
			opaqueIndices_.push_back(start + 2);
			opaqueIndices_.push_back(start + 0);
			opaqueIndices_.push_back(start + 2);
			opaqueIndices_.push_back(start + 3);
		};

	// chunk size x, y, z
	const int dims[3] = { CHUNK_SIZE, CHUNK_SIZE_Y, CHUNK_SIZE };

	// sweep axis d (0 = x, 1 = y, 2 = z)
	for (int d = 0; d < 3; ++d)
	{
		// get other two axes for mask
		int u = (d + 1) % 3;
		int v = (d + 2) % 3;

		// unit step
		int q[3] = { 0, 0, 0 };
		q[d] = 1;

		std::vector<MaskCell> mask(dims[u] * dims[v]);
		int x[3] = { 0, 0, 0 };

		// when x[d] = -1, a is outside chunk and b is slice 0
		// when x[d] = dims[d]-1, b is outside and a is last slice
		for (x[d] = -1; x[d] < dims[d]; ++x[d])
		{
			// build mask for this slice
			int n = 0;
			for (x[v] = 0; x[v] < dims[v]; ++x[v])
			{
				for (x[u] = 0; x[u] < dims[u]; ++x[u], ++n)
				{
					BlockID a = BlockID::Air;
					BlockID b = BlockID::Air;

					if (x[d] >= 0)
						a = chunkData_.getBlockID(x[0], x[1], x[2]);
					if (x[d] < dims[d] - 1)
						b = chunkData_.getBlockID(x[0] + q[0], x[1] + q[1], x[2] + q[2]);

					// check which side is opaque
					bool aSolid = (x[d] >= 0) && isOpaque(a);
					bool bSolid = (x[d] < dims[d] - 1) && isOpaque(b);

					MaskCell cell{};
					cell.valid = false;

					// face between a and b if one is solid and the other is transparent
					if (aSolid && isTransparent(x[0] + q[0], x[1] + q[1], x[2] + q[2]))
					{
						cell.valid = true;
						cell.id = a;
						cell.dir = (d == 0) ? FaceDir::PosX : (d == 1) ? FaceDir::PosY : FaceDir::PosZ;
						getBlockTile(a, cell.tileX, cell.tileY, cell.dir);
					}
					else if (bSolid && isTransparent(x[0], x[1], x[2]))
					{
						cell.valid = true;
						cell.id = b;
						cell.dir = (d == 0) ? FaceDir::NegX : (d == 1) ? FaceDir::NegY : FaceDir::NegZ;
						getBlockTile(b, cell.tileX, cell.tileY, cell.dir);
					}

					mask[n] = cell;
				} // end for
			} // end for

			// greedy merge rectangles on mask
			n = 0;
			for (int j = 0; j < dims[v]; ++j)
			{
				for (int i = 0; i < dims[u]; )
				{
					MaskCell c = mask[n];
					if (!c.valid) 
					{ 
						++i; 
						++n; 
						continue; 
					}

					int w = 1;
					while (i + w < dims[u]) 
					{
						MaskCell c2 = mask[n + w];
						if (!c2.valid || c2.id != c.id || c2.dir != c.dir ||
							c2.tileX != c.tileX || c2.tileY != c.tileY)
						{
							break;
						}
						++w;
					} // end while

					int h = 1;
					bool stop = false;
					while (j + h < dims[v] && !stop) 
					{
						for (int k = 0; k < w; ++k) 
						{
							MaskCell c2 = mask[n + k + h * dims[u]];
							if (!c2.valid || c2.id != c.id || c2.dir != c.dir ||
								c2.tileX != c.tileX || c2.tileY != c.tileY)
							{
								stop = true;
								break;
							}
						} // end for
						if (!stop) ++h;
					} // end while

					int du[3] = { 0,0,0 };
					int dv[3] = { 0,0,0 };
					du[u] = w;
					dv[v] = h;

					int base[3] = { 0,0,0 };
					base[u] = i;
					base[v] = j;
					base[d] = x[d] + 1;

					glm::ivec3 p0(base[0], base[1], base[2]);
					glm::ivec3 p1(base[0] + dv[0], base[1] + dv[1], base[2] + dv[2]);
					glm::ivec3 p2(base[0] + du[0] + dv[0], base[1] + du[1] + dv[1], base[2] + du[2] + dv[2]);
					glm::ivec3 p3(base[0] + du[0], base[1] + du[1], base[2] + du[2]);

					const bool neg = (c.dir == FaceDir::NegX || c.dir == FaceDir::NegY || c.dir == FaceDir::NegZ);
					if (neg)
					{
						emitQuad(p0, p3, p2, p1, c.dir, c.tileX, c.tileY);
					}
					else
					{
						emitQuad(p0, p1, p2, p3, c.dir, c.tileX, c.tileY);
					}

					// clear mask region
					for (int yy = 0; yy < h; ++yy)
					{
						for (int xx = 0; xx < w; ++xx)
						{
							mask[n + xx + yy * dims[u]].valid = false;
						} // end for
					} // end for

					i += w;
					n += w;
				} // end for
			} // end for
		} // end for
	} // end for

	// water
	waterVertices_.clear();
	waterIndices_.clear();

	auto addWaterQuad = [&](int x0, int y, int z0, int w, int h)
		{
			// water surface height
			float yPos = static_cast<float>(y) + 0.90f;

			// pos, in chunk local space
			glm::vec3 p0{ x0, yPos, z0 };
			glm::vec3 p1{ x0 + w,yPos, z0 };
			glm::vec3 p2{ x0 + w, yPos, z0 + h };
			glm::vec3 p3{ x0, yPos, z0 + h };

			// texture coord
			int tileX;
			int tileY;
			getBlockTile(BlockID::Water, tileX, tileY, std::nullopt);

			uint32_t start = static_cast<uint32_t>(waterVertices_.size());

			VertexWater v0; v0.pos = p0;
			VertexWater v1; v1.pos = p1;
			VertexWater v2; v2.pos = p2;
			VertexWater v3; v3.pos = p3;

			waterVertices_.push_back(v0);
			waterVertices_.push_back(v1);
			waterVertices_.push_back(v2);
			waterVertices_.push_back(v3);

			// two triangles
			waterIndices_.push_back(start + 0);
			waterIndices_.push_back(start + 1);
			waterIndices_.push_back(start + 2);
			waterIndices_.push_back(start + 0);
			waterIndices_.push_back(start + 2);
			waterIndices_.push_back(start + 3);
		};

	for (int y = 0; y < CHUNK_SIZE_Y; ++y)
	{
		bool mask[CHUNK_SIZE][CHUNK_SIZE] = {};

		for (int x = 0; x < CHUNK_SIZE; ++x)
		{
			for (int z = 0; z < CHUNK_SIZE; ++z)
			{
				BlockID block = chunkData_.getBlockID(x, y, z);

				// skip opaque blocks
				if (block != BlockID::Water) continue;

				BlockID above = (y + 1) > CHUNK_SIZE_Y ?
					BlockID::Air :
					chunkData_.getBlockID(x, y + 1, z);

				if (above != BlockID::Water)
				{
					mask[x][z] = true;
				}
			} // end for
		} // end for

		for (int z = 0; z < CHUNK_SIZE; ++z)
		{
			for (int x = 0; x < CHUNK_SIZE; ++x)
			{
				if (!mask[x][z])
				{
					continue;
				}

				int w = 1;
				while (x + w < CHUNK_SIZE && mask[x + w][z])
				{
					w++;
				} // end while

				int h = 1;
				bool stop = false;
				while (z + h < CHUNK_SIZE && !stop)
				{
					for (int i = 0; i < w; ++i)
					{
						if (!mask[x + i][z + h])
						{
							stop = true;
							break;
						}
					} // end for

					if (!stop)
					{
						h++;
					}
				} // end while

				addWaterQuad(x, y, z, w, h);

				for (int dz = 0; dz < h; ++dz)
				{
					for (int dx = 0; dx < w; ++dx)
					{
						mask[x + dx][z + dz] = false;
					} // end for
				} // end for
			} // end for
		} // end for
	} // end for

	// update rendered block count
	renderedBlockCount_ = computeRenderedBlockCount();
} // end of buildChunkMesh()

bool ChunkMesh::isTransparent(int x, int y, int z)
{
	// check for outside chunk (= air)
	if (x < 0 || x >= CHUNK_SIZE ||
		y < 0 || y >= CHUNK_SIZE_Y ||
		z < 0 || z >= CHUNK_SIZE)
	{
		return true;
	}

	BlockID id = chunkData_.getBlockID(x, y, z);

	if (id == BlockID::Tree_Leaf || id == BlockID::Water)
	{
		return true;
	}

	return id == BlockID::Air;
} // end of isTransparent()

uint32_t ChunkMesh::computeRenderedBlockCount()
{
	uint32_t count = 0;

	for (int x = 0; x < CHUNK_SIZE; ++x)
		for (int y = 0; y < CHUNK_SIZE_Y; ++y)
			for (int z = 0; z < CHUNK_SIZE; ++z)
			{
				BlockID id = chunkData_.getBlockID(x, y, z);
				if (id == BlockID::Air || id == BlockID::Water) continue;

				// count block if ANY face is visible
				if (isTransparent(x + 1, y, z) ||
					isTransparent(x - 1, y, z) ||
					isTransparent(x, y + 1, z) ||
					isTransparent(x, y - 1, z) ||
					isTransparent(x, y, z + 1) ||
					isTransparent(x, y, z - 1))
				{
					++count;
				}
			} // end for

	return count;
} // end of computeRenderedBlockCount()

glm::vec2 ChunkMesh::atlasUV(const glm::vec2& localUV, int tileX, int tileY)
{
	float tileW = 1.0f / ATLAS_COLS;
	float tileH = 1.0f / ATLAS_ROWS;

	return glm::vec2(
		(tileX + localUV.x) * tileW,
		(tileY + localUV.y) * tileH
	);
} // end of atlasUV()

void ChunkMesh::getBlockTile(BlockID id, int& tileX, int& tileY, std::optional<FaceDir> face)
{
	switch (id)
	{
	case BlockID::Dirt:
		tileX = 5; tileY = tileYFromTop(7);
		break;
	case BlockID::Grass:
		switch (*face)
		{
		case FaceDir::PosY: // top
			tileX = 14; tileY = tileYFromTop(5); // grass
			break;
		case FaceDir::NegY: // bottom
			tileX = 5; tileY = tileYFromTop(7); // dirt
			break;
		default: // sides
			tileX = 14; tileY = tileYFromTop(2); // grass
			break;
		}
		break;
	case BlockID::Stone:
		tileX = 8; tileY = tileYFromTop(2);
		break;
	case BlockID::Tree_Leaf:
		tileX = 8; tileY = tileYFromTop(10);
		break;
	case BlockID::Tree_Trunk:
		switch (*face)
		{
		case FaceDir::PosY: // top
		case FaceDir::NegY: // bottom
			tileX = 11; tileY = tileYFromTop(10);
			break;
		default: // sides
			tileX = 10; tileY = tileYFromTop(10);
			break;
		}
		break;
	case BlockID::Glow_Block:
		tileX = 10; tileY = tileYFromTop(14);
		break;
	case BlockID::Sand:
		tileX = 0, tileY = tileYFromTop(15);
		break;
	case BlockID::Water:
		tileX = 2; tileY = tileYFromTop(0);
		break;
	default:
		tileX = 0; tileY = tileYFromTop(0);
		break;
	}
} // end of getBlockTile()