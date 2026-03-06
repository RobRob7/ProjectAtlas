#include "chunk_pass_vk.h"

#include "vulkan_main.h"

#include "render_inputs.h"
#include "chunk_manager.h"
#include "i_chunk_mesh_gpu.h"

#include <glm/gtc/matrix_transform.hpp>
#include <cstring>
#include <stdexcept>

struct ChunkFrameUBO
{
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec2 screenSize;
    float ambientStrength;
    float _pad0;
};

struct ChunkPush
{
    glm::vec3 chunkOrigin;
    float _pad0;
};

//--- PUBLIC ---//
ChunkPassVk::ChunkPassVk(VulkanMain& vk) : vk_(vk) {}

