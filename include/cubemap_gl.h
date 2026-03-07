#ifndef CUBEMAP_GL_H
#define CUBEMAP_GL_H

#include "i_cubemap.h"

#include "ubo_gl.h"

#include <glm/glm.hpp>

#include <string_view>
#include <array>
#include <cstdint>
#include <memory>

class Texture;
class Shader;

class CubemapGL final : public ICubemap
{
public:
    CubemapGL(const std::array<std::string_view, 6>& textures = Cubemap_Constants::DEFAULT_FACES);
    ~CubemapGL() override;

    void init() override;
    void render(const RenderContext& ctx, const glm::mat4& view, const glm::mat4& projection, const float time = -1.0) override;

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
