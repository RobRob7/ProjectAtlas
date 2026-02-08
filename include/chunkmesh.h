#ifndef CHUNKMESH_H
#define CHUNKMESH_H

#include "chunkdata.h"
#include "data.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <vector>
#include <array>
#include <cstdint>
#include <cstddef>
#include <optional>

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
    ChunkMesh(int chunkX, int chunkY);
    ~ChunkMesh();

    void uploadChunkMesh();

    void renderChunkOpaque();
    void renderChunkWater();

    void setBlock(int x, int y, int z, BlockID id);
    BlockID getBlock(int x, int y, int z) const;
    ChunkData& getChunk();
    void rebuild();

    uint32_t getRenderedBlockCount() const;

private:
    // water
    std::vector<VertexWater> waterVertices_;
    std::vector<uint32_t> waterIndices_;
    uint32_t waterVao_{};
    uint32_t waterVbo_{};
    uint32_t waterEbo_{};
    int32_t waterIndexCount_{};

    // opaque
	std::vector<Vertex> opaqueVertices_;
	std::vector<uint32_t> opaqueIndices_;
    uint32_t opaqueVao_{};
    uint32_t opaqueVbo_{};
    uint32_t opaqueEbo_{};
    int32_t opaqueIndexCount_{};

    ChunkData chunkData_;

    uint32_t renderedBlockCount_ = 0;
private:
	void buildChunkMesh();
	bool isTransparent(int x, int y, int z);

    // atlas
    glm::vec2 atlasUV(const glm::vec2& localUV, int tileX, int tileY);
    void getBlockTile(BlockID id, int& tileX, int& tileY, std::optional<FaceDir> face);
};

#endif
