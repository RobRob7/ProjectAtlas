#include "waterpass.h"

//--- PUBLIC ---//
WaterPass::~WaterPass()
{
	destroyGL();
} // end of destructor

void WaterPass::init()
{
    // NOTHING FOR NOW
} // end of init()

void WaterPass::resize(int w, int h)
{
    if (w <= 0 || h <= 0) return;
    if (w == width_ && h == height_) return;
    
    destroyTargets();
    width_ = w;
    height_ = h;
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


//--- PRIVATE ---//
void WaterPass::createTargets()
{
    glCreateFramebuffers(1, &reflFBO_);

    glCreateFramebuffers(1, &refrFBO_);

    // reflection
    glCreateTextures(GL_TEXTURE_2D, 1, &reflColorTex_);
    glTextureStorage2D(reflColorTex_, 1, GL_RGBA8, width_ / 1, height_ / 1);
    glTextureParameteri(reflColorTex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(reflColorTex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(reflColorTex_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(reflColorTex_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCreateRenderbuffers(1, &reflDepthRBO_);
    glNamedRenderbufferStorage(reflDepthRBO_, GL_DEPTH_COMPONENT24, width_ / 1, height_ / 1);

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

    waterReflectionPass(in);
    waterRefractionPass(in);

    glDisable(GL_DEPTH_TEST);
} // end of waterPass()

void WaterPass::waterReflectionPass(const RenderInputs& in)
{
    glBindFramebuffer(GL_FRAMEBUFFER, reflFBO_);
    glViewport(0, 0, width_ / 1, height_ / 1);
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

void WaterPass::waterRefractionPass(const RenderInputs& in)
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