#ifndef FOG_PASS_H
#define FOG_PASS_H

#include "constants.h"
#include "bindings.h"

#include "ubo_gl.h"

#include <cstdint>
#include <memory>

class Shader;
struct RenderSettings;

class FogPass
{
public:
    FogPass();
    ~FogPass();

    void init();
    void resize(int w, int h);

    void render(
        uint32_t sceneColorTex, 
        uint32_t sceneDepthTex,
        uint32_t shadowMapTex,
        Fog_Constants::FogPassUBO& ubo
    );

    uint32_t getOutputTex() const { return outputTex_; }

private:
    void destroyGL();
private:
    int width_{};
    int height_{};

    uint32_t fsVao_{};
    uint32_t fbo_{};
    uint32_t outputTex_{};

    std::unique_ptr<Shader> shader_;

    UBOGL uboBuffer_{ TO_API_FORM(FogPassBinding::UBO) };
};

#endif
