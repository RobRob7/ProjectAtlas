#include "gbuffer_pass.h"

#include "constants.h"

#include "chunk_pass_gl.h"
#include "shader.h"
#include "render_inputs.h"

#include <glad/glad.h>

#include <stdexcept>
#include <memory>

//--- PUBLIC ---//
GBufferPass::GBufferPass() = default;

GBufferPass::~GBufferPass()
{
	destroyGL();
} // end of destructor

void GBufferPass::init()
{
	destroyGL();

	gBufferShader_ = std::make_unique<Shader>("gbuffer/gbuffer.vert", "gbuffer/gbuffer.frag");

	ubo_.init<sizeof(GbufferUBO)>();
} // end of init()

void GBufferPass::resize(int w, int h)
{
	if (w <= 0 || h <= 0) return;
	if (w == width_ && h == height_) return;

	destroyTargets();
	width_	= w;
	height_ = h;
	createTargets();
} // end of resize()

void GBufferPass::render(
	ChunkPassGL& chunk,
	const RenderInputs& in,
	const glm::mat4& view,
	const glm::mat4& proj)
{
	if (!gBufferShader_ || !fbo_ || width_ <= 0 || height_ <= 0) return;

	// bind ubo
	ubo_.bind();

	glViewport(0, 0, width_, height_);

	glEnable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gBufferShader_->use();
	gbufferUBO_.u_view = view;
	gbufferUBO_.u_proj = proj;
	ubo_.update(&gbufferUBO_, sizeof(gbufferUBO_));

	chunk.renderOpaqueOffscreen(
		ubo_, 
		&gbufferUBO_, 
		sizeof(gbufferUBO_), 
		gbufferUBO_.u_chunkOrigin,
		in, 
		view, 
		proj
	);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
} // end of render()


//--- PRIVATE ---//
void GBufferPass::createTargets()
{
	glCreateFramebuffers(1, &fbo_);

	// normal texture
	glCreateTextures(GL_TEXTURE_2D, 1, &gNormalTexture_);
	glTextureStorage2D(gNormalTexture_, 1, GL_RGB16F, width_, height_);
	glTextureParameteri(gNormalTexture_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(gNormalTexture_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(gNormalTexture_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(gNormalTexture_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// depth texture
	glCreateTextures(GL_TEXTURE_2D, 1, &gDepthTexture_);
	glTextureStorage2D(gDepthTexture_, 1, GL_DEPTH_COMPONENT24, width_, height_);
	glTextureParameteri(gDepthTexture_, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTextureParameteri(gDepthTexture_, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTextureParameteri(gDepthTexture_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(gDepthTexture_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// attach
	glNamedFramebufferTexture(fbo_, GL_COLOR_ATTACHMENT0, gNormalTexture_, 0);
	glNamedFramebufferTexture(fbo_, GL_DEPTH_ATTACHMENT, gDepthTexture_, 0);

	// draw buffers
	const GLenum drawBufs[] = { GL_COLOR_ATTACHMENT0 };
	glNamedFramebufferDrawBuffers(fbo_, 1, drawBufs);

	if (glCheckNamedFramebufferStatus(fbo_, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("GBuffer FBO incomplete!");
	}
} // end of createTargets()

void GBufferPass::destroyTargets()
{
	if (fbo_)
	{
		glDeleteFramebuffers(1, &fbo_);
		fbo_ = 0;
	}
	if (gNormalTexture_)
	{
		glDeleteTextures(1, &gNormalTexture_);
		gNormalTexture_ = 0;
	}
	if (gDepthTexture_)
	{
		glDeleteTextures(1, &gDepthTexture_);
		gDepthTexture_ = 0;
	}
} // end of destroyTargets()

void GBufferPass::destroyGL()
{
	destroyTargets();

	width_ = 0;
	height_ = 0;
} // end of destroyGL()