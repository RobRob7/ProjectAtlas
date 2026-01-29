#include "chunkmesh.h"

//--- PUBLIC ---//
ChunkMesh::ChunkMesh(int chunkX, int chunkY, Shader& opaqueShader, Texture& texture, Shader& waterShader)
	: chunkData_(chunkX, chunkY), opaqueShader_(opaqueShader), texture_(texture), waterShader_(waterShader)
{
	// OPAQUE
	// create VAO + buffers
	glCreateVertexArrays(1, &opaqueVao_);
	glCreateBuffers(1, &opaqueVbo_);
	glCreateBuffers(1, &opaqueEbo_);

	// attach buffers to vao
	glVertexArrayVertexBuffer(opaqueVao_, 0, opaqueVbo_, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(opaqueVao_, opaqueEbo_);

	// position
	glEnableVertexArrayAttrib(opaqueVao_, 0);
	glVertexArrayAttribFormat(opaqueVao_, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
	glVertexArrayAttribBinding(opaqueVao_, 0, 0);

	// normal
	glEnableVertexArrayAttrib(opaqueVao_, 1);
	glVertexArrayAttribFormat(opaqueVao_, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
	glVertexArrayAttribBinding(opaqueVao_, 1, 0);

	// uv
	glEnableVertexArrayAttrib(opaqueVao_, 2);
	glVertexArrayAttribFormat(opaqueVao_, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
	glVertexArrayAttribBinding(opaqueVao_, 2, 0);


	// WATER
	// create VAO + buffers
	glCreateVertexArrays(1, &waterVao_);
	glCreateBuffers(1, &waterVbo_);
	glCreateBuffers(1, &waterEbo_);

	// attach buffers to vao
	glVertexArrayVertexBuffer(waterVao_, 0, waterVbo_, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(waterVao_, waterEbo_);

	// position
	glEnableVertexArrayAttrib(waterVao_, 0);
	glVertexArrayAttribFormat(waterVao_, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
	glVertexArrayAttribBinding(waterVao_, 0, 0);

	// normal
	glEnableVertexArrayAttrib(waterVao_, 1);
	glVertexArrayAttribFormat(waterVao_, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
	glVertexArrayAttribBinding(waterVao_, 1, 0);

	// uv
	glEnableVertexArrayAttrib(waterVao_, 2);
	glVertexArrayAttribFormat(waterVao_, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
	glVertexArrayAttribBinding(waterVao_, 2, 0);

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
	opaqueShader_.use();
	opaqueShader_.setInt("u_atlas", 0);

	waterShader_.use();
	waterShader_.setInt("u_atlas", 0);

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
		waterVertices_.size() * sizeof(Vertex),
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
	glm::mat4 model = glm::translate(glm::mat4(1.0f), 
		glm::vec3(chunkData_.m_chunkX * CHUNK_SIZE, 0.0f, chunkData_.m_chunkZ * CHUNK_SIZE));

	opaqueShader_.setMat4("u_model", model);

	glBindVertexArray(opaqueVao_);
	glDrawElements(GL_TRIANGLES, opaqueIndexCount_, GL_UNSIGNED_INT, nullptr);
} // end of render()

void ChunkMesh::renderChunkOpaque(Shader& shader)
{
	glm::mat4 model = glm::translate(glm::mat4(1.0f),
		glm::vec3(chunkData_.m_chunkX * CHUNK_SIZE, 0.0f, chunkData_.m_chunkZ * CHUNK_SIZE));

	shader.setMat4("u_model", model);

	glBindVertexArray(opaqueVao_);
	glDrawElements(GL_TRIANGLES, opaqueIndexCount_, GL_UNSIGNED_INT, nullptr);
} // end of render()

void ChunkMesh::renderChunkWater(Shader& waterShader)
{
	if (waterIndexCount_ <= 0) return;

	glm::mat4 model = glm::translate(
		glm::mat4(1.0f),
		glm::vec3(chunkData_.m_chunkX * CHUNK_SIZE, 0.0f, chunkData_.m_chunkZ * CHUNK_SIZE)
	);

	waterShader.setMat4("u_model", model);

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
	opaqueVertices_.clear();
	opaqueIndices_.clear();
	opaqueVertices_.reserve(CHUNK_SIZE * CHUNK_SIZE * 24);
	opaqueIndices_.reserve(CHUNK_SIZE * CHUNK_SIZE * 24);

	renderedBlockCount_ = 0;

	// indexing
	auto addFace = [&](const std::array<Vertex, 4> faceVerts,
						const std::array<uint32_t, 6> faceIndices,
						int bx, int by, int bz,
						BlockID id,
						FaceDir faceDir)
		{
			uint32_t startIndex = static_cast<uint32_t>(opaqueVertices_.size());

			int tileX;
			int tileY;
			getBlockTile(id, tileX, tileY, faceDir);

			// offset block local face positions by block position in world
			for (int i = 0; i < 4; ++i)
			{
				Vertex v = faceVerts[i];
				v.pos += glm::vec3(bx, by, bz);
				v.uv = atlasUV(v.uv, tileX, tileY);
				opaqueVertices_.push_back(v);
			}
			for (int i = 0; i < 6; ++i)
			{
				opaqueIndices_.push_back(startIndex + faceIndices[i]);
			}
		};

	// opaque
	for (int x = 0; x < CHUNK_SIZE; ++x)
	{
		for (int y = 0; y < CHUNK_SIZE_Y; ++y)
		{
			for (int z = 0; z < CHUNK_SIZE; ++z)
			{
				BlockID id = chunkData_.getBlockID(x, y, z);

				// air, skip
				if (id == BlockID::Air) continue;

				// water, skip
				if (id == BlockID::Water)
				{
					continue;
				}

				// count bool
				bool emittedAnyFace = false;

				// +X
				if (isTransparent(x + 1, y, z))
				{
					addFace(FACE_POS_X, FACE_INDICES, x, y, z, id, FaceDir::PosX);
					emittedAnyFace = true;
				}
				// -X
				if (isTransparent(x - 1, y, z))
				{
					addFace(FACE_NEG_X, FACE_INDICES, x, y, z, id, FaceDir::NegX);
					emittedAnyFace = true;
				}
				// +Y
				if (isTransparent(x, y + 1, z))
				{
					addFace(FACE_POS_Y, FACE_INDICES, x, y, z, id, FaceDir::PosY);
					emittedAnyFace = true;
				}
				// -Y
				if (isTransparent(x, y - 1, z))
				{
					addFace(FACE_NEG_Y, FACE_INDICES, x, y, z, id, FaceDir::NegY);
					emittedAnyFace = true;
				}
				// +Z
				if (isTransparent(x, y, z + 1))
				{
					addFace(FACE_POS_Z, FACE_INDICES, x, y, z, id, FaceDir::PosZ);
					emittedAnyFace = true;
				}
				// -Z
				if (isTransparent(x, y, z - 1))
				{
					addFace(FACE_NEG_Z, FACE_INDICES, x, y, z, id, FaceDir::NegZ);
					emittedAnyFace = true;
				}

				// count update
				if (emittedAnyFace)
				{
					++renderedBlockCount_;
				}
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

			Vertex v0; v0.pos = p0; v0.normal = { 0,1,0 }; v0.uv = atlasUV({ 0,0 }, tileX, tileY);
			Vertex v1; v1.pos = p1; v1.normal = { 0,1,0 }; v1.uv = atlasUV({ 1,0 }, tileX, tileY);
			Vertex v2; v2.pos = p2; v2.normal = { 0,1,0 }; v2.uv = atlasUV({ 1,1 }, tileX, tileY);
			Vertex v3; v3.pos = p3; v3.normal = { 0,1,0 }; v3.uv = atlasUV({ 0,1 }, tileX, tileY);

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