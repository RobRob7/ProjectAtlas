#ifndef FXAAPASS_H
#define FXAAPASS_H

#include "shader.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <cstdint>
#include <optional>

class FXAAPass
{
public:
    ~FXAAPass();

    void init();
    void resize(int w, int h);
    void destroyGL();
    void render(uint32_t sceneColorTex, int w, int h);

    void setEnabled(bool e);
    bool enabled() const;

    void setSharpnessQuality(float v);
    void setEdgeThresholdMax(float v);
    void setEdgeThresholdMin(float v);

    uint32_t getOutputTex() const;

private:
    bool enabled_{ true };

    float edgeSharpnessQuality_{ 8.0f };
    float edgeThresholdMax_{ 0.125f };
    float edgeThresholdMin_{ 0.0625f };

    uint32_t fsVao_{};
    uint32_t fxaaFBO_{};
    uint32_t fxaaColorTex_{};

    std::optional<Shader> shader_;
};

#endif
