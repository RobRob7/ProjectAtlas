#ifndef FOGPASS_H
#define FOGPASS_H

class Shader;

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>

class FogPass
{
public:
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
    float fogEnd_{ 200.0f };

    uint32_t fsVao_{};
    std::unique_ptr<Shader> shader_;
};

#endif
