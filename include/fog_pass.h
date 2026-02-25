#ifndef FOGPASS_H
#define FOGPASS_H

#include "ubo_gl.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>

struct FogPassUBO
{
    float u_near;
    float u_far;
    glm::vec2 _pad0;

    glm::vec3 u_fogColor;
    float _pad1;

    float u_fogStart;
    float u_fogEnd;
    float u_ambStr;
    float _pad2;
};

class Shader;

class FogPass
{
public:
    FogPass();
    ~FogPass();

    void init();
    void render(uint32_t sceneColorTex, uint32_t sceneDepthTex,
        float nearPlane, float farPlane, float ambStr);

    void setFogColor(glm::vec3 v);
    void setFogStart(float v);
    void setFogEnd(float v);

private:
    void destroyGL();
private:
    glm::vec3 fogColor_{ 1.0f, 1.0f, 1.0f };
    float fogStart_{ 50.0f };
    float fogEnd_{ 2000.0f };

    uint32_t fsVao_{};
    std::unique_ptr<Shader> shader_;

    UBOGL ubo_{ UBOBinding::FogPass };
    FogPassUBO fogPassUBO_;
};

#endif
