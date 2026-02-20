#ifndef CHUNKMESHDATA_H
#define CHUNKMESHDATA_H

#include <glm/glm.hpp>

#include <vector>
#include <cstdint>

// world opaque vertices
// LAYOUT (32u bits)
// 0  - 1   : UV corner index
// 2  - 6   : tileY
// 7  - 11  : tileX
// 12 - 14  : normal index
// 15 - 18  : x pos
// 19 - 27  : y pos
// 28 - 31  : z pos
struct Vertex
{
    uint32_t sample;
};

struct VertexWater
{
    glm::vec3 pos;
};

struct ChunkMeshData
{
    // opaque
    std::vector<Vertex> opaqueVertices;
    std::vector<uint32_t> opaqueIndices;
    int32_t opaqueIndexCount = 0;

    // water
    std::vector<VertexWater> waterVertices;
    std::vector<uint32_t> waterIndices;
    int32_t waterIndexCount = 0;

    uint32_t renderedBlockCount = 0;
};

#endif
