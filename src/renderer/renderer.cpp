#include "renderer.h"

//--- PUBLIC ---//
void Renderer::init()
{
	gbuffer_.init();
	debugPass_.init();
    ssaoPass_.init();

    glCreateFramebuffers(1, &forwardFBO_);
    glCreateTextures(GL_TEXTURE_2D, 1, &forwardColorTex_);
    glCreateTextures(GL_TEXTURE_2D, 1, &forwardDepthTex_);

    waterPass_.init();
    fxaaPass_.init();
    fogPass_.init();
    presentPass_.init();
} // end of init()

void Renderer::resize(int w, int h)
{
    if (w <= 0 || h <= 0) return;
    if (w == width_ && h == height_) return;

    width_ = w;
    height_ = h;

    gbuffer_.resize(width_, height_);
    ssaoPass_.resize(width_, height_);
    fxaaPass_.resize(width_, height_);
    waterPass_.resize(width_, height_);

    fxaaResize();
} // end of resize()

void Renderer::renderFrame(const RenderInputs& in)
{
    if (!in.world || !in.camera || !in.light || !in.skybox || !in.crosshair) return;

    const glm::mat4 view = in.camera->getViewMatrix();
    const float aspect = (height_ > 0)
        ? (static_cast<float>(width_) / static_cast<float>(height_))
        : 1.0f;
    const glm::mat4 proj = in.camera->getProjectionMatrix(aspect);


    // --------------- PASSES --------------- //
    // gbuffer pass
    gbuffer_.render(*in.world, view, proj);

    // ssao pass
    if (renderSettings_.useSSAO)
    {
        glm::mat4 invProj = glm::inverse(proj);
        ssaoPass_.render(gbuffer_.getNormalTexture(), gbuffer_.getDepthTexture(), proj, invProj);
    }

    // debug pass
    if (renderSettings_.debugMode == DebugMode::Normals || renderSettings_.debugMode == DebugMode::Depth)
    {
        debugPass_.render(
            gbuffer_.getNormalTexture(),
            gbuffer_.getDepthTexture(),
            in.camera->getNearPlane(),
            in.camera->getFarPlane(),
            (renderSettings_.debugMode == DebugMode::Normals) ? 1 : 2);
        return;
    }

    // water pass
    waterPass_.render(in);
    // --------------- END PASSES --------------- //


    // --------------- FORWARD RENDER --------------- //
    in.world->update(in.camera->getCameraPosition());

    glBindFramebuffer(GL_FRAMEBUFFER, forwardFBO_);
    glViewport(0, 0, width_, height_);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update uniforms of world shader
    auto& worldShader = in.world->getOpaqueShader();
    worldShader->use();
    worldShader->setFloat("u_ambientStrength", in.world->getAmbientStrength());
    worldShader->setVec3("u_viewPos", in.camera->getCameraPosition());
    worldShader->setVec3("u_lightPos", in.light->getPosition());
    worldShader->setVec3("u_lightColor", in.light->getColor());

    // ssao
    worldShader->setVec2("u_screenSize", glm::vec2(width_, height_));
    worldShader->setBool("u_useSSAO", renderSettings_.useSSAO);
    worldShader->setInt("u_ssao", 3);
    if (renderSettings_.useSSAO)
    {
        glBindTextureUnit(3, ssaoPass_.aoBlurTexture());
    }
    else
    {
        glBindTextureUnit(3, 0);
    }

    // update uniforms of water shader
    auto& waterShader = in.world->getWaterShader();
    waterShader->use();
    waterShader->setFloat("u_ambientStrength", in.world->getAmbientStrength());
    waterShader->setVec3("u_viewPos", in.camera->getCameraPosition());
    waterShader->setVec3("u_lightPos", in.light->getPosition());
    waterShader->setVec3("u_lightColor", in.light->getColor());
    waterShader->setFloat("u_near", in.camera->getNearPlane());
    waterShader->setFloat("u_far", in.camera->getFarPlane());

    waterShader->setInt("u_reflectionTex", 4);
    waterShader->setInt("u_refractionTex", 5);
    waterShader->setInt("u_refractionDepthTex", 6);
    waterShader->setInt("u_dudvTex", 7);
    waterShader->setFloat("u_time", in.time);

    // bind textures
    glBindTextureUnit(4, waterPass_.getReflColorTex());
    glBindTextureUnit(5, waterPass_.getRefrColorTex());
    glBindTextureUnit(6, waterPass_.getRefrDepthTex());
    glBindTextureUnit(7, waterPass_.getDuDVTex());

    // render objects (non-UI)
    in.world->renderOpaque(view, proj);
    in.world->renderWater(view, proj);
    in.light->render(view, proj);
    in.skybox->render(view, proj, in.time);
    // --------------- END FORWARD RENDER --------------- //


    // --------------- POST-PROCESSING --------------- //
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width_, height_);
    glDisable(GL_DEPTH_TEST);

    // FXAA
    uint32_t finalColorTex = forwardColorTex_;
    if (renderSettings_.useFXAA)
    {
        fxaaPass_.render(forwardColorTex_, width_, height_);
        finalColorTex = fxaaPass_.getOutputTex();
    }

    // FOG
    fogPass_.setFogColor(renderSettings_.fogSettings.color);
    fogPass_.setFogStart(renderSettings_.fogSettings.start);
    fogPass_.setFogEnd(renderSettings_.fogSettings.end);
    if (renderSettings_.useFog)
    {
        fogPass_.render(finalColorTex, forwardDepthTex_, width_, height_,
            in.camera->getNearPlane(), in.camera->getFarPlane(), in.world->getAmbientStrength());
    }
    else
    {
        presentPass_.render(finalColorTex, width_, height_);
    }
    // --------------- END POST-PROCESSING --------------- //


    // --------------- UI ELEMENTS --------------- //
    in.crosshair->render();
    // --------------- END UI ELEMENTS --------------- //
} // end of renderFrame()

RenderSettings& Renderer::settings()
{
    return renderSettings_;
} // end of settings()


//--- PRIVATE ---//
void Renderer::fxaaResize()
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
} // end of fxaaResize()
