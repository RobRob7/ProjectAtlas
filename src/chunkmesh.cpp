#include "chunkmesh.h"

//--- PUBLIC ---//
ChunkMesh::ChunkMesh() :
	chunkData_(0, 0)
{
	buildChunkMesh();
} // end of constructor

void ChunkMesh::uploadChunkMesh()
{
	chunkShader_ = { "chunk/chunk.vert", "chunk/chunk.frag" };

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

	indexCount_ = static_cast<int32_t>(indices_.size());
} // end of uploadChunkMesh()

void ChunkMesh::renderChunk(const glm::mat4& view, const glm::mat4& proj)
{
	chunkShader_.use();

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
						int bx, int by, int bz)
		{
			uint32_t startIndex = static_cast<uint32_t>(vertices_.size());

			// offset block local face positions by block position in world
			for (int i = 0; i < 4; ++i)
			{
				Vertex v = faceVerts[i];
				v.pos += glm::vec3(bx, by, bz);
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
					addFace(FACE_POS_X, FACE_INDICES, x, y, z);
				}
				// -X
				if (isAir(x - 1, y, z))
				{
					addFace(FACE_NEG_X, FACE_INDICES, x, y, z);
				}
				// +Y
				if (isAir(x, y + 1, z))
				{
					addFace(FACE_POS_Y, FACE_INDICES, x, y, z);
				}
				// -Y
				if (isAir(x, y - 1, z))
				{
					addFace(FACE_NEG_Y, FACE_INDICES, x, y, z);
				}
				// +Z
				if (isAir(x, y, z + 1))
				{
					addFace(FACE_POS_Z, FACE_INDICES, x, y, z);
				}
				// -Z
				if (isAir(x, y, z - 1))
				{
					addFace(FACE_NEG_Z, FACE_INDICES, x, y, z);
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