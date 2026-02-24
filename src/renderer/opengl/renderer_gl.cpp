#include "renderer_gl.h"

#include "chunk_opaque_pass_gl.h"

#include "chunk_manager.h"
#include "camera.h"
#include "light_gl.h"
#include "cubeMap.h"
#include "crosshair.h"

#include "gbuffer_pass.h"
#include "debug_pass.h"
#include "ssao_pass.h"
#include "fxaa_pass.h"
#include "present_pass.h"
#include "water_pass.h"
#include "fog_pass.h"

#include "render_inputs.h"

#include <glad/glad.h>

#include <stdexcept>

//--- PUBLIC ---//
RendererGL::RendererGL() = default;

RendererGL::~RendererGL()
{
    destroyGL();
} // end of destructor

void RendererGL::init()
{
    destroyGL();

    if (!renderSettings_) renderSettings_ = std::make_unique<RenderSettings>();

    if (!gbuffer_)     gbuffer_ = std::make_unique<GBufferPass>();
    if (!debugPass_)   debugPass_ = std::make_unique<DebugPass>();
    if (!ssaoPass_)    ssaoPass_ = std::make_unique<SSAOPass>();
    if (!fxaaPass_)    fxaaPass_ = std::make_unique<FXAAPass>();
    if (!fogPass_)     fogPass_ = std::make_unique<FogPass>();
    if (!presentPass_) presentPass_ = std::make_unique<PresentPass>();
    if (!waterPass_)   waterPass_ = std::make_unique<WaterPass>();

    if (!chunkOpaque_) chunkOpaque_ = std::make_unique<ChunkOpaquePassGL>();
    chunkOpaque_->init();

	gbuffer_->init();
	debugPass_->init();
    ssaoPass_->init();

    glCreateFramebuffers(1, &forwardFBO_);
    glCreateTextures(GL_TEXTURE_2D, 1, &forwardColorTex_);
    glCreateTextures(GL_TEXTURE_2D, 1, &forwardDepthTex_);

    waterPass_->init();
    fxaaPass_->init();
    fogPass_->init();
    presentPass_->init();
} // end of init()

void RendererGL::resize(int w, int h)
{
    if (w <= 0 || h <= 0) return;
    if (w == width_ && h == height_) return;

    width_ = w;
    height_ = h;

    gbuffer_->resize(width_, height_);
    ssaoPass_->resize(width_, height_);
    fxaaPass_->resize(width_, height_);
    waterPass_->resize(width_, height_);
    presentPass_->resize(width_, height_);

    resizeForwardTargets();
} // end of resize()

void RendererGL::renderFrame(const RenderInputs& in)
{
    if (!in.world || !in.camera || !in.light || !in.skybox || !in.crosshair) return;

    in.world->update(in.camera->getCameraPosition());

    const glm::mat4 view = in.camera->getViewMatrix();
    const float aspect = (height_ > 0)
        ? (static_cast<float>(width_) / static_cast<float>(height_))
        : 1.0f;
    const glm::mat4 proj = in.camera->getProjectionMatrix(aspect);


    // --------------- PASSES --------------- //
    // gbuffer pass
    gbuffer_->render(*chunkOpaque_, in, view, proj);

    // ssao pass
    if (renderSettings_->useSSAO)
    {
        glm::mat4 invProj = glm::inverse(proj);
        ssaoPass_->render(gbuffer_->getNormalTexture(), gbuffer_->getDepthTexture(), proj, invProj);
    }

    // debug pass
    if (renderSettings_->debugMode == DebugMode::Normals || renderSettings_->debugMode == DebugMode::Depth)
    {
        debugPass_->render(
            gbuffer_->getNormalTexture(),
            gbuffer_->getDepthTexture(),
            in.camera->getNearPlane(),
            in.camera->getFarPlane(),
            (renderSettings_->debugMode == DebugMode::Normals) ? 1 : 2);
        return;
    }

    // water pass
    waterPass_->render(*chunkOpaque_, in);
    // --------------- END PASSES --------------- //


    // --------------- FORWARD RENDER --------------- //
    glBindFramebuffer(GL_FRAMEBUFFER, forwardFBO_);
    glViewport(0, 0, width_, height_);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (renderSettings_->useSSAO)
    {
        glBindTextureUnit(3, ssaoPass_->aoBlurTexture());
    }
    else
    {
        glBindTextureUnit(3, 0);
    }

    // bind textures
    glBindTextureUnit(4, waterPass_->getReflColorTex());
    glBindTextureUnit(5, waterPass_->getRefrColorTex());
    glBindTextureUnit(6, waterPass_->getRefrDepthTex());
    glBindTextureUnit(7, waterPass_->getDuDVTex());
    glBindTextureUnit(8, waterPass_->getNormalTex());

    // update opaque + water shader
    chunkOpaque_->updateShader(in, *renderSettings_, width_, height_);

    // render objects (non-UI)
    chunkOpaque_->renderOpaque(in, view, proj, width_, height_);
    chunkOpaque_->renderWater(in, view, proj, width_, height_);
    in.light->render(view, proj);
    in.skybox->render(view, proj, in.time);
    // --------------- END FORWARD RENDER --------------- //


    // --------------- POST-PROCESSING --------------- //
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width_, height_);
    glDisable(GL_DEPTH_TEST);

    // FXAA
    uint32_t finalColorTex = forwardColorTex_;
    if (renderSettings_->useFXAA)
    {
        fxaaPass_->render(forwardColorTex_);
        finalColorTex = fxaaPass_->getOutputTex();
    }

    // FOG
    fogPass_->setFogColor(renderSettings_->fogSettings.color);
    fogPass_->setFogStart(renderSettings_->fogSettings.start);
    fogPass_->setFogEnd(renderSettings_->fogSettings.end);
    if (renderSettings_->useFog)
    {
        fogPass_->render(finalColorTex, forwardDepthTex_,
            in.camera->getNearPlane(), in.camera->getFarPlane(), in.world->getAmbientStrength());
    }
    else
    {
        presentPass_->render(finalColorTex);
    }
    // --------------- END POST-PROCESSING --------------- //


    // --------------- UI ELEMENTS --------------- //
    in.crosshair->render();
    // --------------- END UI ELEMENTS --------------- //
} // end of renderFrame()

RenderSettings& RendererGL::settings()
{
    return *renderSettings_;
} // end of settings()


//--- PRIVATE ---//
void RendererGL::destroyGL()
{
    if (forwardFBO_)
    {
        glDeleteFramebuffers(1, &forwardFBO_);
        forwardFBO_ = 0;
    }

    if (forwardColorTex_)
    {
        glDeleteTextures(1, &forwardColorTex_);
        forwardColorTex_ = 0;
    }

    if (forwardDepthTex_)
    {
        glDeleteTextures(1, &forwardDepthTex_);
        forwardDepthTex_ = 0;
    }
} // end of destroyGL()

void RendererGL::resizeForwardTargets()
{
    // recreate textures
    if (forwardColorTex_)
    {
        glDeleteTextures(1, &forwardColorTex_);
        forwardColorTex_ = 0;
    }
    if (forwardDepthTex_)
    {
        glDeleteTextures(1, &forwardDepthTex_);
        forwardDepthTex_ = 0;
    }
    glCreateTextures(GL_TEXTURE_2D, 1, &forwardColorTex_);

    glTextureStorage2D(forwardColorTex_, 1, GL_RGBA8, width_, height_);
    glTextureParameteri(forwardColorTex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(forwardColorTex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(forwardColorTex_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(forwardColorTex_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCreateTextures(GL_TEXTURE_2D, 1, &forwardDepthTex_);

    glTextureStorage2D(forwardDepthTex_, 1, GL_DEPTH_COMPONENT24, width_, height_);
    glTextureParameteri(forwardDepthTex_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(forwardDepthTex_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(forwardDepthTex_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(forwardDepthTex_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glNamedFramebufferTexture(forwardFBO_, GL_COLOR_ATTACHMENT0, forwardColorTex_, 0);
    glNamedFramebufferTexture(forwardFBO_, GL_DEPTH_ATTACHMENT, forwardDepthTex_, 0);

    GLenum drawBuf = GL_COLOR_ATTACHMENT0;
    glNamedFramebufferDrawBuffers(forwardFBO_, 1, &drawBuf);

    // check FBO
    if (glCheckNamedFramebufferStatus(forwardFBO_, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("FBO 'forwardFBO_' is incomplete!");
    }
} // end of resizeForwardTargets()
