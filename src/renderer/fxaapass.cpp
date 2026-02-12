#include "fxaapass.h"

#include "shader.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <stdexcept>

//--- PUBLIC ---//
FXAAPass::~FXAAPass()
{
	destroyGL();
} // end of destructor

void FXAAPass::init()
{
	shader_ = std::make_unique<Shader>("fxaapass/fxaa.vert", "fxaapass/fxaa.frag");

	shader_->use();
	shader_->setInt("u_sceneColorTex", 0);

	glCreateVertexArrays(1, &fsVao_);
} // end of init()

void FXAAPass::resize(int w, int h)
{
	if (w <= 0 || h <= 0) return;
	if (w == width_ && h == height_) return;

	destroyTargets();
	width_ = w;
	height_ = h;
	createTargets();

	shader_->use();
	shader_->setVec2("u_inverseScreenSize", glm::vec2(1.0f / static_cast<float>(width_), 1.0f / static_cast<float>(height_)));
} // end of resize()

void FXAAPass::render(uint32_t sceneColorTex)
{
	if (!shader_ || !sceneColorTex || width_ <= 0 || height_ <= 0)
		return;

	const GLboolean prevDepth = glIsEnabled(GL_DEPTH_TEST);

	glViewport(0, 0, width_, height_);

	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, fxaaFBO_);
	glBindVertexArray(fsVao_);

	shader_->use();
	shader_->setFloat("u_edgeSharpnessQuality", edgeSharpnessQuality_);
	shader_->setFloat("u_edgeThresholdMax", edgeThresholdMax_);
	shader_->setFloat("u_edgeThresholdMin", edgeThresholdMin_);

	glBindTextureUnit(0, sceneColorTex);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (prevDepth) glEnable(GL_DEPTH_TEST);
} // end of render()

void FXAAPass::setSharpnessQuality(float v)
{ 
	edgeSharpnessQuality_ = v;
} // end of setSubpix()

void FXAAPass::setEdgeThresholdMax(float v)
{ 
	edgeThresholdMax_ = v;
} // end of setEdgeThreshold()

void FXAAPass::setEdgeThresholdMin(float v) 
{
	edgeThresholdMin_ = v; 
} // end of setEdgeThresholdMin()

uint32_t FXAAPass::getOutputTex() const
{
	return fxaaColorTex_;
} // end of getOutputTex()


//--- PRIVATE ---//
void FXAAPass::createTargets()
{
	glCreateFramebuffers(1, &fxaaFBO_);
	glCreateTextures(GL_TEXTURE_2D, 1, &fxaaColorTex_);

	glTextureStorage2D(fxaaColorTex_, 1, GL_RGBA8, width_, height_);
	glTextureParameteri(fxaaColorTex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(fxaaColorTex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(fxaaColorTex_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(fxaaColorTex_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glNamedFramebufferTexture(fxaaFBO_, GL_COLOR_ATTACHMENT0, fxaaColorTex_, 0);
	GLenum buf = GL_COLOR_ATTACHMENT0;
	glNamedFramebufferDrawBuffers(fxaaFBO_, 1, &buf);

	if (glCheckNamedFramebufferStatus(fxaaFBO_, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		throw std::runtime_error("FXAA FBO incomplete!");
	}
} // end of createTargets()

void FXAAPass::destroyTargets()
{
	if (fxaaFBO_)
	{
		glDeleteFramebuffers(1, &fxaaFBO_);
		fxaaFBO_ = 0;
	}
	if (fxaaColorTex_)
	{
		glDeleteTextures(1, &fxaaColorTex_);
		fxaaColorTex_ = 0;
	}
} // end of destroyTargets()

void FXAAPass::destroyGL()
{
	destroyTargets();

	if (fsVao_)
	{
		glDeleteVertexArrays(1, &fsVao_);
		fsVao_ = 0;
	}

	width_ = 0;
	height_ = 0;
} // end of destroyGL()
