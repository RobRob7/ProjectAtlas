#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "texture.h"
#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <cstdint>
#include <optional>

// cubemap default
inline const std::array<std::string, 6> DEFAULT_FACES = { {
    "texture/cubemap/space_alt/right.png",
    "texture/cubemap/space_alt/left.png",
    "texture/cubemap/space_alt/top.png",
    "texture/cubemap/space_alt/bottom.png",
    "texture/cubemap/space_alt/front.png",
    "texture/cubemap/space_alt/back.png"
} };

const float SkyboxVertices[] =
{
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

class CubeMap
{
public:
	// constructor
    CubeMap(const std::array<std::string, 6>& textures = DEFAULT_FACES);
    // destructor
    ~CubeMap();

    // disallow copy
    CubeMap(const CubeMap&) = delete;
    // disallow copy assignment
    CubeMap& operator=(const CubeMap&) = delete;

    void init();

    // render cubemap
    void render(const glm::mat4& view, const glm::mat4& projection, const float time = -1.0) const;

private:
    // cubemap shader
    std::optional<Shader> shader_;
    // cubemap texture
    Texture texture_;
    uint32_t& cubemapTexture_;
    // cubemap VAO, VBO
    uint32_t vao_{};
    uint32_t vbo_{};
};

#endif
