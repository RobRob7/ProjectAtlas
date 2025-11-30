#ifndef CHUNKMESH_H
#define CHUNKMESH_H

#include "chunkdata.h"
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include <vector>
#include <cstdint>

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};

inline constexpr std::array<uint32_t, 6> FACE_INDICES = { 0, 1, 2, 0, 2, 3 };

// +X (right) face
inline constexpr std::array<Vertex, 4> FACE_POS_X = { {
    {{1, 0, 0}, {1, 0, 0}, {0, 0}},
    {{1, 1, 0}, {1, 0, 0}, {0, 1}},
    {{1, 1, 1}, {1, 0, 0}, {1, 1}},
    {{1, 0, 1}, {1, 0, 0}, {1, 0}}
} };

// -X (left) face
inline constexpr std::array<Vertex, 4> FACE_NEG_X = { {
    {{0, 0, 1}, {-1, 0, 0}, {0, 0}},
    {{0, 1, 1}, {-1, 0, 0}, {0, 1}},
    {{0, 1, 0}, {-1, 0, 0}, {1, 1}},
    {{0, 0, 0}, {-1, 0, 0}, {1, 0}}
} };

// +Y (top) face
inline constexpr std::array<Vertex, 4> FACE_POS_Y = { {
    {{0, 1, 0}, {0, 1, 0}, {0, 0}},
    {{1, 1, 0}, {0, 1, 0}, {1, 0}},
    {{1, 1, 1}, {0, 1, 0}, {1, 1}},
    {{0, 1, 1}, {0, 1, 0}, {0, 1}}
} };

// -Y (bottom) face
inline constexpr std::array<Vertex, 4> FACE_NEG_Y = { {
    {{0, 0, 1}, {0, -1, 0}, {0, 0}},
    {{1, 0, 1}, {0, -1, 0}, {1, 0}},
    {{1, 0, 0}, {0, -1, 0}, {1, 1}},
    {{0, 0, 0}, {0, -1, 0}, {0, 1}}
} };

// +Z (front) face
inline constexpr std::array<Vertex, 4> FACE_POS_Z = { {
    {{0, 0, 1}, {0, 0, 1}, {0, 0}},
    {{0, 1, 1}, {0, 0, 1}, {0, 1}},
    {{1, 1, 1}, {0, 0, 1}, {1, 1}},
    {{1, 0, 1}, {0, 0, 1}, {1, 0}}
} };

// -Z (back) face
inline constexpr std::array<Vertex, 4> FACE_NEG_Z = { {
    {{1, 0, 0}, {0, 0, -1}, {0, 0}},
    {{1, 1, 0}, {0, 0, -1}, {0, 1}},
    {{0, 1, 0}, {0, 0, -1}, {1, 1}},
    {{0, 0, 0}, {0, 0, -1}, {1, 0}}
} };


class ChunkMesh
{
public:
	ChunkMesh();
    void uploadChunkMesh();
    void renderChunk(const glm::mat4& view, const glm::mat4& proj);

private:
	std::vector<Vertex> vertices_;
	std::vector<uint32_t> indices_;
	ChunkData chunkData_;
    // render
    Shader chunkShader_;
    uint32_t vao_;
    uint32_t vbo_;
    uint32_t ebo_;
    int32_t indexCount_;
private:
	void buildChunkMesh();
	bool isAir(int x, int y, int z);
};

#endif
