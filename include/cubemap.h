#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "ubo_gl.h"

#include <glm/glm.hpp>

#include <string_view>
#include <array>
#include <cstdint>
#include <memory>

class Texture;
class Shader;

extern const std::array<float, 108> SkyboxVertices;
extern const std::array<std::string_view, 6> DEFAULT_FACES;

class CubeMap
{
public:
    CubeMap(const std::array<std::string_view, 6>& textures = DEFAULT_FACES);
    ~CubeMap();

    // disallow copy
    CubeMap(const CubeMap&) = delete;
    // disallow copy assignment
    CubeMap& operator=(const CubeMap&) = delete;

    void init();

    // render cubemap
    void render(const glm::mat4& view, const glm::mat4& projection, const float time = -1.0);

private:
    void destroyGL();
private:
    std::unique_ptr<Shader> shader_;
    std::unique_ptr<Texture> texture_;

    uint32_t vao_{};
    uint32_t vbo_{};
    UBOGL ubo_{ UBOBinding::Cubemap };

    std::array<std::string_view, 6> faces_;
};

#endif
