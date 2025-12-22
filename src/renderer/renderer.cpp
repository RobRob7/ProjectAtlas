#include "renderer.h"

//--- PUBLIC ---//
void Renderer::init()
{
	gbuffer_.init();
	debugPass_.init();
    ssaoPass_.init();
} // end of init()

void Renderer::resize(float w, float h)
{
    if (w <= 0 || h <= 0) return;
    if (w == width_ && h == height_) return;

    width_ = w;
    height_ = h;

    gbuffer_.resize(width_, height_);
    ssaoPass_.resize(width_, height_);
} // end of resize()

void Renderer::renderFrame(const RenderInputs& in)
{
    if (!in.world || !in.camera || !in.light || !in.skybox || !in.crosshair) return;

    const glm::mat4 view = in.camera->getViewMatrix();
    const float aspect = (height_ > 0) ? (width_ / height_) : 1.0f;
    const glm::mat4 proj = in.camera->getProjectionMatrix(aspect);

    // gbuffer pass
    gbuffer_.render(*in.world, view, proj);

    // ssao pass
    if (in.useSSAO)
    {
        glm::mat4 invProj = glm::inverse(proj);
        ssaoPass_.render(gbuffer_.getNormalTexture(), gbuffer_.getDepthTexture(), proj, invProj);
    }

    // debug pass
    if (in.debugMode == DebugMode::Normals || in.debugMode == DebugMode::Depth)
    {
        debugPass_.render(
            gbuffer_.getNormalTexture(),
            gbuffer_.getDepthTexture(),
            in.camera->getNearPlane(),
            in.camera->getFarPlane(),
            (in.debugMode == DebugMode::Normals) ? 1 : 2);
        return;
    }

    // forward render
    in.world->update(in.camera->getCameraPosition());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width_, height_);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update uniforms of world shader
    auto& worldShader = in.world->getShader();
    worldShader->use();
    worldShader->setVec3("u_viewPos", in.camera->getCameraPosition());
    worldShader->setVec3("u_lightPos", in.light->getPosition());
    worldShader->setVec3("u_lightColor", in.light->getColor());
    // ssao
    worldShader->setVec2("u_screenSize", glm::vec2(static_cast<int>(width_), static_cast<int>(height_)));
    worldShader->setBool("u_useSSAO", in.useSSAO);
    worldShader->setInt("u_ssao", 3);
    if (in.useSSAO)
    {
        glBindTextureUnit(3, ssaoPass_.aoBlurTexture());
    }
    else
    {
        glBindTextureUnit(3, 0);
    }

    // render objects
    in.world->render(view, proj);
    in.light->render(view, proj);
    in.skybox->render(view, proj, in.time);
    in.crosshair->render();
} // end of renderFrame()