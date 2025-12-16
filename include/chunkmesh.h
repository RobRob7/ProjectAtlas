#ifndef CHUNKMESH_H
#define CHUNKMESH_H

#include "chunkdata.h"
#include "shader.h"
#include "texture.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include <vector>
#include <array>
#include <cstdint>
#include <cstddef>

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
    ChunkMesh(int chunkX, int chunkY, Shader& shader, Texture& texture);
    ~ChunkMesh();

    void uploadChunkMesh();
    void renderChunk();

    void setBlock(int x, int y, int z, BlockID id);
    BlockID getBlock(int x, int y, int z) const;
    ChunkData& ChunkMesh::getChunk();
    void rebuild();

private:
	std::vector<Vertex> vertices_;
	std::vector<uint32_t> indices_;
	ChunkData chunkData_;
    // shader + texture
    Shader& shader_;
    Texture& texture_;
    // render
    uint32_t vao_{};
    uint32_t vbo_{};
    uint32_t ebo_{};
    int32_t indexCount_{};
private:
	void buildChunkMesh();
	bool isTransparent(int x, int y, int z);

    // atlas
    glm::vec2 atlasUV(const glm::vec2& localUV, int tileX, int tileY);
    void getBlockTile(BlockID id, FaceDir face, int& tileX, int& tileY);
};

#endif
