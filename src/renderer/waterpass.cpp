#include "waterpass.h"

#include "shader.h"

//--- PUBLIC ---//
WaterPass::~WaterPass()
{
	destroyGL();
} // end of destructor

void WaterPass::init()
{
    dudvTex_.emplace("dudv.png");
    dudvTex_->setWrapRepeat();
    normalTex_.emplace("waternormal.png");
    normalTex_->setWrapRepeat();
} // end of init()

void WaterPass::resize(int w, int h)
{
    if (w <= 0 || h <= 0) return;
    if (w == fullW_ && h == fullH_) return;
    
    destroyTargets();
    fullW_ = w;
    fullH_ = h;
    width_  = std::max(1, w / factor_);
    height_ = std::max(1, h / factor_);
    createTargets();
} // end of resize()

void WaterPass::destroyGL()
{
    destroyTargets();

    width_ = 0;
    height_ = 0;
} // end of destroyGL()

void WaterPass::render(const RenderInputs& in)
{
    waterPass(in);
} // end of render()

const uint32_t& WaterPass::getReflColorTex() const
{
    return reflColorTex_;
} // end of getReflColorTex()

const uint32_t& WaterPass::getRefrColorTex() const
{
    return refrColorTex_;
} // end of getRefrColorTex()

const uint32_t& WaterPass::getRefrDepthTex() const
{
    return refrDepthTex_;
} // end of getRefrDepthTex()

const uint32_t& WaterPass::getDuDVTex() const
{
    return dudvTex_->m_ID;
} // end of getDuDVTex()

const uint32_t& WaterPass::getNormalTex() const
{
    return normalTex_->m_ID;
} // end of getNormalTex()


//--- PRIVATE ---//
void WaterPass::createTargets()
{
    glCreateFramebuffers(1, &reflFBO_);

    glCreateFramebuffers(1, &refrFBO_);

    // reflection
    glCreateTextures(GL_TEXTURE_2D, 1, &reflColorTex_);
    glTextureStorage2D(reflColorTex_, 1, GL_RGBA8, width_, height_);
    glTextureParameteri(reflColorTex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(reflColorTex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(reflColorTex_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(reflColorTex_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCreateRenderbuffers(1, &reflDepthRBO_);
    glNamedRenderbufferStorage(reflDepthRBO_, GL_DEPTH_COMPONENT24, width_, height_);

    glNamedFramebufferTexture(reflFBO_, GL_COLOR_ATTACHMENT0, reflColorTex_, 0);
    glNamedFramebufferRenderbuffer(reflFBO_, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, reflDepthRBO_);

    GLenum drawBuf = GL_COLOR_ATTACHMENT0;
    glNamedFramebufferDrawBuffers(reflFBO_, 1, &drawBuf);

    // refraction
    glCreateTextures(GL_TEXTURE_2D, 1, &refrColorTex_);
    glTextureStorage2D(refrColorTex_, 1, GL_RGBA8, width_, height_);
    glTextureParameteri(refrColorTex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(refrColorTex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(refrColorTex_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(refrColorTex_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCreateTextures(GL_TEXTURE_2D, 1, &refrDepthTex_);
    glTextureStorage2D(refrDepthTex_, 1, GL_DEPTH_COMPONENT24, width_, height_);
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
        throw std::runtime_error("FBO 'reflFBO_' is incomplete!");
    }
    if (glCheckNamedFramebufferStatus(refrFBO_, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error("FBO 'refrFBO_' is incomplete!");
    }
} // end of createTargets()

void WaterPass::destroyTargets()
{
    if (reflFBO_)
    {
        glDeleteFramebuffers(1, &reflFBO_);
        reflFBO_ = 0;
    }
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
    if (refrFBO_)
    {
        glDeleteFramebuffers(1, &refrFBO_);
        refrFBO_ = 0;
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
} // end of destroyTargets()

void WaterPass::waterPass(const RenderInputs& in)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CLIP_DISTANCE0);
    waterReflectionPass(in);
    waterRefractionPass(in);

    // restore framebuffer + viewport
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, fullW_, fullH_);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CLIP_DISTANCE0);
} // end of waterPass()

void WaterPass::waterReflectionPass(const RenderInputs& in)
{
    glBindFramebuffer(GL_FRAMEBUFFER, reflFBO_);
    glViewport(0, 0, width_, height_);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // build reflected view matrix
    float waterHeight = static_cast<float>(SEA_LEVEL) + 0.9f;
    Camera& camera = *in.camera;
    float distance = 2.0f * (camera.getCameraPosition().y - waterHeight);
    camera.getCameraPosition().y -= distance;
    camera.invertPitch();
    glm::mat4 reflView = camera.getViewMatrix();

    // set clip plane (clip everything below water)
    glm::vec4 clipPlane{ 0, 1, 0, -(waterHeight)};
    auto& opaqueShader = in.world->getOpaqueShader();
    opaqueShader->use();
    opaqueShader->setVec4("u_clipPlane", clipPlane);

    // disable SSAO
    opaqueShader->setBool("u_useSSAO", false);

    const float aspect = (fullH_ > 0)
        ? (static_cast<float>(fullW_) / static_cast<float>(fullH_))
        : 1.0f;
    const glm::mat4 proj = in.camera->getProjectionMatrix(aspect);

    opaqueShader->setVec3("u_viewPos", camera.getCameraPosition());

    // render objects (non-UI)
    in.world->renderOpaque(*opaqueShader, reflView, proj);
    in.light->render(reflView, proj);
    in.skybox->render(reflView, proj);

    // restore camera
    camera.getCameraPosition().y += distance;
    camera.invertPitch();
} // end of waterReflectionPass()

void WaterPass::waterRefractionPass(const RenderInputs& in)
{
    glBindFramebuffer(GL_FRAMEBUFFER, refrFBO_);
    glViewport(0, 0, width_, height_);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set clip plane (clip everything above water)
    float waterHeight = static_cast<float>(SEA_LEVEL) + 0.9f;
    glm::vec4 clipPlane{ 0, -1, 0, (waterHeight) };
    auto& opaqueShader = in.world->getOpaqueShader();
    opaqueShader->use();
    opaqueShader->setVec4("u_clipPlane", clipPlane);

    // disable SSAO
    opaqueShader->setBool("u_useSSAO", false);

    const glm::mat4 view = in.camera->getViewMatrix();

    const float aspect = (fullH_ > 0)
        ? (static_cast<float>(fullW_) / static_cast<float>(fullH_))
        : 1.0f;
    const glm::mat4 proj = in.camera->getProjectionMatrix(aspect);

    opaqueShader->setVec3("u_viewPos", in.camera->getCameraPosition());

    // render objects (non-UI)
    in.world->renderOpaque(*opaqueShader, view, proj);
    in.light->render(view, proj);
} // end of waterRefractionPass()