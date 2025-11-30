#ifndef CHUNKMESH_H
#define CHUNKMESH_H

#include "chunkdata.h"
#include "shader.h"
#include "texture.h"

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

inline constexpr int ATLAS_COLS = 32;
inline constexpr int ATLAS_ROWS = 32;

enum class FaceDir {
    PosX, NegX,
    PosY, NegY,
    PosZ, NegZ
};

// helper for when texture is flipped vertically
inline int tileYFromTop(int rowFromTop) 
{
    return ATLAS_ROWS - 1 - rowFromTop;
}

struct ChunkCoord
{
    int x;
    int z;

    bool operator==(const ChunkCoord& other) const noexcept
    {
        return x == other.x && z == other.z;
    }
};

struct ChunkCoordHash
{
    size_t operator()(const ChunkCoord& c) const noexcept
    {
        return std::hash<int>()(c.x) ^ (std::hash<int>()(c.z) << 1);
    }
};


class ChunkMesh
{
public:
	ChunkMesh();
    ChunkMesh(int chunkX, int chunkY);
    ~ChunkMesh();

    void uploadChunkMesh();
    void renderChunk(const glm::mat4& view, const glm::mat4& proj);

private:
	std::vector<Vertex> vertices_;
	std::vector<uint32_t> indices_;
	ChunkData chunkData_;
    // texture
    Texture texture_{"blocks.png", true};
    // render
    Shader chunkShader_;
    uint32_t vao_ = 0;
    uint32_t vbo_ = 0;
    uint32_t ebo_ = 0;
    int32_t indexCount_;
private:
	void buildChunkMesh();
	bool isAir(int x, int y, int z);

    // atlas
    glm::vec2 atlasUV(const glm::vec2& localUV, int tileX, int tileY);
    void getBlockTile(BlockID id, FaceDir face, int& tileX, int& tileY);
};

#endif
