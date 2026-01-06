#include "chunkmesh.h"

//--- PUBLIC ---//
ChunkMesh::ChunkMesh(int chunkX, int chunkY, Shader& shader, Texture& texture)
	: chunkData_(chunkX, chunkY), shader_(shader), texture_(texture)
{
	// create VAO + buffers
	glCreateVertexArrays(1, &vao_);
	glCreateBuffers(1, &vbo_);
	glCreateBuffers(1, &ebo_);

	// attach buffers to vao
	glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(Vertex));
	glVertexArrayElementBuffer(vao_, ebo_);

	// position
	glEnableVertexArrayAttrib(vao_, 0);
	glVertexArrayAttribFormat(vao_, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
	glVertexArrayAttribBinding(vao_, 0, 0);

	// normal
	glEnableVertexArrayAttrib(vao_, 1);
	glVertexArrayAttribFormat(vao_, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
	glVertexArrayAttribBinding(vao_, 1, 0);

	// uv
	glEnableVertexArrayAttrib(vao_, 2);
	glVertexArrayAttribFormat(vao_, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
	glVertexArrayAttribBinding(vao_, 2, 0);

	buildChunkMesh();
	uploadChunkMesh();
} // end of other constructor

ChunkMesh::~ChunkMesh()
{
	if (vao_)
	{
		glDeleteVertexArrays(1, &vao_);
		vao_ = 0;
	}
	if (vbo_)
	{
		glDeleteBuffers(1, &vbo_);
		vbo_ = 0;
	}
	if (ebo_)
	{
		glDeleteBuffers(1, &ebo_);
		ebo_ = 0;
	}
} // end of destructor

void ChunkMesh::uploadChunkMesh()
{
	shader_.use();
	shader_.setInt("u_atlas", 0);

	// reupload into vbo
	glNamedBufferData(
		vbo_,
		vertices_.size() * sizeof(Vertex),
		vertices_.empty() ? nullptr : vertices_.data(),
		GL_STATIC_DRAW
	);

	// reupload into ebo
	glNamedBufferData(
		ebo_,
		indices_.size() * sizeof(uint32_t),
		indices_.empty() ? nullptr : indices_.data(),
		GL_STATIC_DRAW
	);

	indexCount_ = static_cast<int32_t>(indices_.size());
} // end of uploadChunkMesh()

void ChunkMesh::renderChunk()
{
	glm::mat4 model = glm::translate(glm::mat4(1.0f), 
		glm::vec3(chunkData_.m_chunkX * CHUNK_SIZE, 0.0f, chunkData_.m_chunkZ * CHUNK_SIZE));

	shader_.setMat4("u_model", model);

	glBindVertexArray(vao_);
	glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
} // end of render()

void ChunkMesh::renderChunk(Shader& shader)
{
	glm::mat4 model = glm::translate(glm::mat4(1.0f),
		glm::vec3(chunkData_.m_chunkX * CHUNK_SIZE, 0.0f, chunkData_.m_chunkZ * CHUNK_SIZE));

	shader.setMat4("u_model", model);

	glBindVertexArray(vao_);
	glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
} // end of render()

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
	vertices_.clear();
	indices_.clear();
	vertices_.reserve(CHUNK_SIZE * CHUNK_SIZE * 24);
	indices_.reserve(CHUNK_SIZE * CHUNK_SIZE * 24);

	renderedBlockCount_ = 0;

	// indexing
	auto addFace = [&](const std::array<Vertex, 4> faceVerts,
						const std::array<uint32_t, 6> faceIndices,
						int bx, int by, int bz,
						BlockID id,
						FaceDir faceDir)
		{
			uint32_t startIndex = static_cast<uint32_t>(vertices_.size());

			int tileX;
			int tileY;
			getBlockTile(id, faceDir, tileX, tileY);

			// offset block local face positions by block position in world
			for (int i = 0; i < 4; ++i)
			{
				Vertex v = faceVerts[i];
				v.pos += glm::vec3(bx, by, bz);
				v.uv = atlasUV(v.uv, tileX, tileY);
				vertices_.push_back(v);
			}
			for (int i = 0; i < 6; ++i)
			{
				indices_.push_back(startIndex + faceIndices[i]);
			}
		};

	for (int x = 0; x < CHUNK_SIZE; ++x)
	{
		for (int y = 0; y < CHUNK_SIZE_Y; ++y)
		{
			for (int z = 0; z < CHUNK_SIZE; ++z)
			{
				BlockID id = chunkData_.getBlockID(x, y, z);

				// air, skip
				if (id == BlockID::Air) continue;

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

	if (id == BlockID::Tree_Leaf)
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

void ChunkMesh::getBlockTile(BlockID id, FaceDir face, int& tileX, int& tileY)
{
	switch (id)
	{
	case BlockID::Dirt:
		tileX = 5; tileY = tileYFromTop(7);
		break;
	case BlockID::Grass:
		switch (face)
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
		switch (face)
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