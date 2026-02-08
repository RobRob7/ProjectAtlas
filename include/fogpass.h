#ifndef FOGPASS_H
#define FOGPASS_H

#include "shader.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <cstdint>
#include <optional>

class FogPass
{
public:
    ~FogPass();

    void init();
    void destroyGL();
    void render(uint32_t sceneColorTex, uint32_t sceneDepthTex, int w, int h,
        float nearPlane, float farPlane, float ambStr);

    void setFogColor(glm::vec3 v);
    void setFogStart(float v);
    void setFogEnd(float v);

private:
    glm::vec3 fogColor_{ 1.0f, 1.0f, 1.0f };
    float fogStart_{ 50.0f };
    float fogEnd_{ 200.0f };

    uint32_t fsVao_{};
    std::optional<Shader> shader_;
};

#endif
