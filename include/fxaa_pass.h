#ifndef FXAAPASS_H
#define FXAAPASS_H

#include "ubo_gl.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <memory>

struct FXAAPassUBO
{
    glm::vec2 u_inverseScreenSize;
    float u_edgeSharpnessQuality;
    float u_edgeThresholdMax;

    float u_edgeThresholdMin;
    float _pad0;
    glm::vec2 _pad1;
};

class Shader;

class FXAAPass
{
public:
    FXAAPass();
    ~FXAAPass();

    void init();
    void resize(int w, int h);
    void render(uint32_t sceneColorTex);

    void setSharpnessQuality(float v);
    void setEdgeThresholdMax(float v);
    void setEdgeThresholdMin(float v);

    uint32_t getOutputTex() const;

private:
    int width_{};
    int height_{};

    float edgeSharpnessQuality_{ 8.0f };
    float edgeThresholdMax_{ 0.125f };
    float edgeThresholdMin_{ 0.0625f };

    uint32_t fsVao_{};
    uint32_t fxaaFBO_{};
    uint32_t fxaaColorTex_{};

    std::unique_ptr<Shader> shader_;

    UBOGL ubo_{ UBOBinding::FXAAPass };
    FXAAPassUBO fxaaPassUBO_;
private:
    void createTargets();
    void destroyTargets();
    void destroyGL();
};

#endif
