#include "chunkmesh.h"

//--- PUBLIC ---//
ChunkMesh::ChunkMesh()
	: chunkData_(0, 0)
{
	buildChunkMesh();
} // end of constructor

ChunkMesh::ChunkMesh(int chunkX, int chunkY)
	: chunkData_(chunkX, chunkY)
{
	buildChunkMesh();
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
	// texture
	texture_.setupTexture();
	chunkShader_ = { "chunk/chunk.vert", "chunk/chunk.frag" };
	chunkShader_.use();
	chunkShader_.setInt("u_atlas", 0);

	glCreateVertexArrays(1, &vao_);
	glCreateBuffers(1, &vbo_);
	glCreateBuffers(1, &ebo_);

	// vbo
	glNamedBufferData(vbo_, vertices_.size() * sizeof(Vertex), vertices_.data(), GL_STATIC_DRAW);
	// ebo
	glNamedBufferData(ebo_, indices_.size() * sizeof(uint32_t), indices_.data(), GL_STATIC_DRAW);
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

	//// texture
	//texture_.setupTexture();

	indexCount_ = static_cast<int32_t>(indices_.size());
} // end of uploadChunkMesh()

void ChunkMesh::renderChunk(const glm::mat4& view, const glm::mat4& proj)
{
	chunkShader_.use();
	glBindTextureUnit(0, texture_.m_ID);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), 
		glm::vec3(chunkData_.m_chunkX * CHUNK_SIZE, 0.0f, chunkData_.m_chunkZ * CHUNK_SIZE));

	chunkShader_.setMat4("u_model", model);
	chunkShader_.setMat4("u_view", view);
	chunkShader_.setMat4("u_proj", proj);

	glBindVertexArray(vao_);
	glDrawElements(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
} // end of render()

//--- PRIVATE ---//
void ChunkMesh::buildChunkMesh()
{
	vertices_.clear();
	indices_.clear();
	vertices_.reserve(10000);
	indices_.reserve(10000);

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
		for (int y = 0; y < CHUNK_SIZE; ++y)
		{
			for (int z = 0; z < CHUNK_SIZE; ++z)
			{
				BlockID id = chunkData_.getBlockID(x, y, z);

				// air, skip
				if (id == 0) continue;

				// +X
				if (isAir(x + 1, y, z))
				{
					addFace(FACE_POS_X, FACE_INDICES, x, y, z, id, FaceDir::PosX);
				}
				// -X
				if (isAir(x - 1, y, z))
				{
					addFace(FACE_NEG_X, FACE_INDICES, x, y, z, id, FaceDir::NegX);
				}
				// +Y
				if (isAir(x, y + 1, z))
				{
					addFace(FACE_POS_Y, FACE_INDICES, x, y, z, id, FaceDir::PosY);
				}
				// -Y
				if (isAir(x, y - 1, z))
				{
					addFace(FACE_NEG_Y, FACE_INDICES, x, y, z, id, FaceDir::NegY);
				}
				// +Z
				if (isAir(x, y, z + 1))
				{
					addFace(FACE_POS_Z, FACE_INDICES, x, y, z, id, FaceDir::PosZ);
				}
				// -Z
				if (isAir(x, y, z - 1))
				{
					addFace(FACE_NEG_Z, FACE_INDICES, x, y, z, id, FaceDir::NegZ);
				}
			} // end for
		} // end for
	} // end for
} // end of buildChunkMesh()

bool ChunkMesh::isAir(int x, int y, int z)
{
	// check for outside chunk (= air)
	if (x < 0 || x >= CHUNK_SIZE ||
		y < 0 || y >= CHUNK_SIZE ||
		z < 0 || z >= CHUNK_SIZE)
	{
		return true;
	}

	return chunkData_.getBlockID(x, y, z) == 0;
} // end of isAir()

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
	case 1: // dirt
		tileX = 5; tileY = tileYFromTop(7);
		break;
	case 2: // dirt (grass)
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
	case 3: // stone
		tileX = 8; tileY = tileYFromTop(2);
		break;
	default:
		tileX = 0; tileY = tileYFromTop(0);
		break;
	}
} // end of getBlockTile()