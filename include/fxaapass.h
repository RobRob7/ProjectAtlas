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
    void resize(float w, float h);
    void destroyGL();
    void render(uint32_t sceneColorTex, int w, int h);

    void setEnabled(bool e);
    bool enabled() const;

    void setSubpix(float v);
    void setEdgeThreshold(float v);
    void setEdgeThresholdMin(float v);

private:
    bool enabled_{ true };

    int maxIterations_{ 12 };

    float subPixelQuality_{ 0.75f };
    float edgeThresholdMax_{ 0.125f };
    float edgeThresholdMin_{ 0.0312f };

    uint32_t fsVao_{};
    std::optional<Shader> shader_;
};

#endif
