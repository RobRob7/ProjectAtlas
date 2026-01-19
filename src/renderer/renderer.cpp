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

    fxaaResize();
} // end of resize()

void Renderer::renderFrame(const RenderInputs& in)
{
    if (!in.world || !in.camera || !in.light || !in.skybox || !in.crosshair) return;

    /*printf("Render Res: %d, %d\n", width_, height_);*/
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

    // forward render
    in.world->update(in.camera->getCameraPosition());

    glBindFramebuffer(GL_FRAMEBUFFER, forwardFBO_);
    glViewport(0, 0, width_, height_);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update uniforms of world shader
    auto& worldShader = in.world->getShader();
    worldShader->use();
    worldShader->setFloat("u_ambientStrength", in.world->getAmbientStrength());
    worldShader->setVec3("u_viewPos", in.camera->getCameraPosition());
    worldShader->setVec3("u_lightPos", in.light->getPosition());
    worldShader->setVec3("u_lightColor", in.light->getColor());
    // ssao
    worldShader->setVec2("u_screenSize", glm::vec2(static_cast<int>(width_), static_cast<int>(height_)));
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

    // render objects (non-UI)
    in.world->render(view, proj);
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
} // end of fxaaResize()