#ifndef FXAA_PASS_H
#define FXAA_PASS_H

#include "constants.h"

#include "bindings.h"

#include "ubo_gl.h"

#include <cstdint>
#include <memory>

class Shader;

class FXAAPass
{
public:
    FXAAPass();
    ~FXAAPass();

    void init();
    void resize(int w, int h);

    void render(uint32_t sceneColorTex);

    uint32_t getOutputTex() const;

private:
    void createTargets();
    void destroyTargets();
    void destroyGL();
private:
    int width_{};
    int height_{};

    uint32_t fsVao_{};
    uint32_t fxaaFBO_{};
    uint32_t fxaaColorTex_{};

    std::unique_ptr<Shader> shader_;

    UBOGL ubo_{ TO_API_FORM(FXAAPassBinding::UBO) };
    FXAA_Constants::FXAAPassUBO fxaaPassUBO_;
};

#endif
