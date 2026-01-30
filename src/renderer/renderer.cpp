#include "renderer.h"

//--- PUBLIC ---//
void Renderer::init()
{
	gbuffer_.init();
	debugPass_.init();
    ssaoPass_.init();

    glCreateFramebuffers(1, &forwardFBO_);
    glCreateTextures(GL_TEXTURE_2D, 1, &forwardColorTex_);
    glCreateRenderbuffers(1, &forwardDepthRBO_);


    // WATER reflection FBO, colorTex, depthRBO
    glCreateFramebuffers(1, &reflFBO_);
    glCreateTextures(GL_TEXTURE_2D, 1, &reflColorTex_);
    glCreateRenderbuffers(1, &reflDepthRBO_);
    // WATER refraction FBO, colorTex, depthTex
    glCreateFramebuffers(1, &refrFBO_);
    glCreateTextures(GL_TEXTURE_2D, 1, &refrColorTex_);
    glCreateTextures(GL_TEXTURE_2D, 1, &refrDepthTex_);

    fxaaPass_.init();
} // end of init()

void Renderer::resize(int w, int h)
{
    if (w <= 0 || h <= 0) return;
    if (w == width_ && h == height_) return;

    width_ = w;
    height_ = h;

    gbuffer_.resize(width_, height_);
    ssaoPass_.resize(width_, height_);

    // water resize
    waterResize();

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
    waterPass(in);

    // forward render
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

    // bind textures
    glBindTextureUnit(4, reflColorTex_);
    glBindTextureUnit(5, refrColorTex_);
    glBindTextureUnit(6, refrDepthTex_);

    // render objects (non-UI)
    in.world->renderOpaque(view, proj);
    in.world->renderWater(view, proj);
    in.light->render(view, proj);
    in.skybox->render(view, proj, in.time);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width_, height_);
    glDisable(GL_DEPTH_TEST);

    // FXAA
    if (renderSettings_.useFXAA)
    {
        fxaaPass_.render(forwardColorTex_, width_, height_);
    }
    else
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, forwardFBO_);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, width_, height_,
            0, 0, width_, height_,
            GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }

    in.crosshair->render();
} // end of renderFrame()

RenderSettings& Renderer::settings()
{
    return renderSettings_;
} // end of settings()


//--- PRIVATE ---//
void Renderer::fxaaResize()
{
    // recreate forwardColorTex_
    if (forwardColorTex_)
    {
        glDeleteTextures(1, &forwardColorTex_);
        forwardColorTex_ = 0;
    }
    glCreateTextures(GL_TEXTURE_2D, 1, &forwardColorTex_);

    glTextureStorage2D(forwardColorTex_, 1, GL_RGBA8, width_, height_);
    glTextureParameteri(forwardColorTex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(forwardColorTex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(forwardColorTex_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(forwardColorTex_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glNamedRenderbufferStorage(forwardDepthRBO_, GL_DEPTH24_STENCIL8, width_, height_);

    glNamedFramebufferTexture(forwardFBO_, GL_COLOR_ATTACHMENT0, forwardColorTex_, 0);
    glNamedFramebufferRenderbuffer(forwardFBO_, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, forwardDepthRBO_);

    GLenum drawBuf = GL_COLOR_ATTACHMENT0;
    glNamedFramebufferDrawBuffers(forwardFBO_, 1, &drawBuf);

    // check FBO
    if (glCheckNamedFramebufferStatus(forwardFBO_, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "FBO 'forwardFBO_' is incomplete!\n";
    }
} // end of fxaaResize()

void Renderer::waterResize()
{
    if (reflColorTex_)
    {
        glDeleteTextures(1, &reflColorTex_);
        reflColorTex_ = 0;
    }
    if (reflDepthRBO_)
    {
        glDeleteRenderbuffers(1, &reflDepthRBO_);
        reflDepthRBO_ = 0;
    }
    if (refrColorTex_)
    {
        glDeleteTextures(1, &refrColorTex_);
        refrColorTex_ = 0;
    }
    if (refrDepthTex_)
    {
        glDeleteTextures(1, &refrDepthTex_);
        refrDepthTex_ = 0;
    }

    // reflection
    glCreateTextures(GL_TEXTURE_2D, 1, &reflColorTex_);
    glTextureStorage2D(reflColorTex_, 1, GL_RGBA8, width_ / 1, height_ / 1);
    glTextureParameteri(reflColorTex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(reflColorTex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(reflColorTex_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(reflColorTex_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCreateRenderbuffers(1, &reflDepthRBO_);
    glNamedRenderbufferStorage(reflDepthRBO_, GL_DEPTH_COMPONENT24, width_ / 1, height_/ 1);

    glNamedFramebufferTexture(reflFBO_, GL_COLOR_ATTACHMENT0, reflColorTex_, 0);
    glNamedFramebufferRenderbuffer(reflFBO_, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, reflDepthRBO_);

    GLenum drawBuf = GL_COLOR_ATTACHMENT0;
    glNamedFramebufferDrawBuffers(reflFBO_, 1, &drawBuf);

    // refraction
    glCreateTextures(GL_TEXTURE_2D, 1, &refrColorTex_);
    glTextureStorage2D(refrColorTex_, 1, GL_RGBA8, width_ / 1, height_ / 1);
    glTextureParameteri(refrColorTex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(refrColorTex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(refrColorTex_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(refrColorTex_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCreateTextures(GL_TEXTURE_2D, 1, &refrDepthTex_);
    glTextureStorage2D(refrDepthTex_, 1, GL_DEPTH_COMPONENT24, width_ / 1, height_ / 1);
    glTextureParameteri(refrDepthTex_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(refrDepthTex_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(refrDepthTex_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(refrDepthTex_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glNamedFramebufferTexture(refrFBO_, GL_COLOR_ATTACHMENT0, refrColorTex_, 0);
    glNamedFramebufferTexture(refrFBO_, GL_DEPTH_ATTACHMENT, refrDepthTex_, 0);

    drawBuf = GL_COLOR_ATTACHMENT0;
    glNamedFramebufferDrawBuffers(refrFBO_, 1, &drawBuf);

    // check FBOs
    if (glCheckNamedFramebufferStatus(reflFBO_, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "FBO 'reflFBO_' is incomplete!\n";
    }
    if (glCheckNamedFramebufferStatus(refrFBO_, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cerr << "FBO 'refrFBO_' is incomplete!\n";
    }
} // end of waterResize()

void Renderer::waterPass(const RenderInputs& in)
{
    glEnable(GL_DEPTH_TEST);

    waterReflectionPass(in);
    waterRefractionPass(in);

    glDisable(GL_DEPTH_TEST);
} // end of waterPass()

void Renderer::waterReflectionPass(const RenderInputs& in)
{
    glBindFramebuffer(GL_FRAMEBUFFER, reflFBO_);
    glViewport(0, 0, width_ / 1, height_/ 1);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // build reflected view matrix
    float waterHeight = 64.9f;
    glm::mat4 view = in.camera->getViewMatrix();
    glm::mat4 R(1.0f);
    R[1][1] = -1.0f;
    R[3][1] = 2.0f * waterHeight;
    glm::mat4 reflView = view * R;

    // set clip plane (clip everything below water)
    glm::vec4 clipPlane{ 0, 1, 0, -waterHeight };
    auto& opaqueShader = in.world->getOpaqueShader();
    opaqueShader->use();
    opaqueShader->setVec4("u_clipPlane", clipPlane);
    opaqueShader->setBool("u_useClipPlane", true);

    const float aspect = (height_ > 0)
        ? (static_cast<float>(width_) / static_cast<float>(height_))
        : 1.0f;
    const glm::mat4 proj = in.camera->getProjectionMatrix(aspect);

    // render objects (non-UI)
    in.world->renderOpaque(*opaqueShader, reflView, proj);
    in.light->render(reflView, proj);
    in.skybox->render(reflView, proj);

    opaqueShader->use();
    opaqueShader->setBool("u_useClipPlane", false);
} // end of waterReflectionPass()

void Renderer::waterRefractionPass(const RenderInputs& in)
{
    glBindFramebuffer(GL_FRAMEBUFFER, refrFBO_);
    glViewport(0, 0, width_ / 1, height_ / 1);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float waterHeight = 64.9f;
    // set clip plane (clip everything above water)
    glm::vec4 clipPlane{ 0, -1, 0, waterHeight };
    auto& opaqueShader = in.world->getOpaqueShader();
    opaqueShader->use();
    opaqueShader->setVec4("u_clipPlane", clipPlane);
    opaqueShader->setBool("u_useClipPlane", true);

    const glm::mat4 view = in.camera->getViewMatrix();
    const float aspect = (height_ > 0)
        ? (static_cast<float>(width_) / static_cast<float>(height_))
        : 1.0f;
    const glm::mat4 proj = in.camera->getProjectionMatrix(aspect);

    opaqueShader->setVec3("u_viewPos", in.camera->getCameraPosition());

    // render objects (non-UI)
    in.world->renderOpaque(*opaqueShader, view, proj);
    in.light->render(view, proj);

    opaqueShader->use();
    opaqueShader->setBool("u_useClipPlane", false);
} // end of waterRefractionPass()