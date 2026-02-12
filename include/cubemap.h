#ifndef CUBEMAP_H
#define CUBEMAP_H

class Texture;
class Shader;

#include <glm/glm.hpp>

#include <string_view>
#include <array>
#include <cstdint>
#include <memory>


extern const std::array<float, 108> SkyboxVertices;
extern const std::array<std::string_view, 6> DEFAULT_FACES;

class CubeMap
{
public:
	// constructor
    CubeMap(const std::array<std::string_view, 6>& textures = DEFAULT_FACES);
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
    void destroyGL();
private:
    // cubemap shader
    std::unique_ptr<Shader> shader_;
    // cubemap texture
    std::unique_ptr<Texture> texture_;

    // cubemap VAO, VBO
    uint32_t vao_{};
    uint32_t vbo_{};

    std::array<std::string_view, 6> faces_;
};

#endif
