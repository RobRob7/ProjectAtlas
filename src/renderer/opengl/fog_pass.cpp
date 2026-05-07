#include "fog_pass.h"

#include "constants.h"
#include "shader.h"

#include <glad/glad.h>

#include <memory>
#include <stdexcept>

using namespace Fog_Constants;

//--- PUBLIC ---//
FogPass::FogPass() = default;

FogPass::~FogPass()
{
	destroyGL();
} // end of destructor

void FogPass::init()
{
	destroyGL();

	shader_ = std::make_unique<Shader>(
		"fogpass/fog.vert", 
		"fogpass/fog.frag"
	);

	uboBuffer_.init<sizeof(FogPassUBO)>();

	glCreateVertexArrays(1, &fsVao_);
} // end of init()

void FogPass::resize(int w, int h)
{
	if (w <= 0 || h <= 0) return;
	if (w == width_ && h == height_) return;

	width_ = w;
	height_ = h;

	if (outputTex_)
	{
		glDeleteTextures(1, &outputTex_);
		outputTex_ = 0;
	}

	if (fbo_ == 0)
	{
		glCreateFramebuffers(1, &fbo_);
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &outputTex_);
	glTextureStorage2D(outputTex_, 1, GL_RGBA8, width_, height_);
	glTextureParameteri(outputTex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(outputTex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(outputTex_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(outputTex_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glNamedFramebufferTexture(fbo_, GL_COLOR_ATTACHMENT0, outputTex_, 0);

	GLenum drawBuf = GL_COLOR_ATTACHMENT0;
	glNamedFramebufferDrawBuffers(fbo_, 1, &drawBuf);

	if (glCheckNamedFramebufferStatus(fbo_, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("FogPass framebuffer is incomplete!");
	}
} // end of resize()

void FogPass::render(
	uint32_t sceneColorTex,
	uint32_t sceneDepthTex,
	uint32_t shadowMapTex,
	FogPassUBO& ubo
)
{
	if (!shader_ || !sceneColorTex || !sceneDepthTex || fsVao_ == 0)
		return;

	// bind ubo
	uboBuffer_.bind();

	// bind textures
	glBindTextureUnit(TO_API_FORM(FogPassBinding::ForwardColorTex), sceneColorTex);
	glBindTextureUnit(TO_API_FORM(FogPassBinding::ForwardDepthTex), sceneDepthTex);
	glBindTextureUnit(TO_API_FORM(FogPassBinding::ShadowMapTex), shadowMapTex);

	const GLboolean prevDepth = glIsEnabled(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
	glViewport(0, 0, width_, height_);
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(fsVao_);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	
	shader_->use();
	uboBuffer_.update(&ubo, sizeof(ubo));

	glDrawArrays(GL_TRIANGLES, 0, 3);

	if (prevDepth) glEnable(GL_DEPTH_TEST);
} // end of render()


//--- PRIVATE ---//
void FogPass::destroyGL()
{
	if (outputTex_)
	{
		glDeleteTextures(1, &outputTex_);
		outputTex_ = 0;
	}

	if (fbo_)
	{
		glDeleteFramebuffers(1, &fbo_);
		fbo_ = 0;
	}

	if (fsVao_)
	{
		glDeleteVertexArrays(1, &fsVao_);
		fsVao_ = 0;
	}
} // end of destroyGL()