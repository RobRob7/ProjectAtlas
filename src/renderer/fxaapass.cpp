#include "fxaapass.h"

//--- PUBLIC ---//
FXAAPass::~FXAAPass()
{
	destroyGL();
} // end of destructor

void FXAAPass::init()
{
	shader_.emplace("fxaapass/fxaa.vert", "fxaapass/fxaa.frag");

	glCreateVertexArrays(1, &fsVao_);
} // end of init()

void FXAAPass::resize(int w, int h)
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

	glCreateFramebuffers(1, &fxaaFBO_);
	glCreateTextures(GL_TEXTURE_2D, 1, &fxaaColorTex_);

	glTextureStorage2D(fxaaColorTex_, 1, GL_RGBA8, w, h);
	glTextureParameteri(fxaaColorTex_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(fxaaColorTex_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(fxaaColorTex_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(fxaaColorTex_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glNamedFramebufferTexture(fxaaFBO_, GL_COLOR_ATTACHMENT0, fxaaColorTex_, 0);
	GLenum buf = GL_COLOR_ATTACHMENT0;
	glNamedFramebufferDrawBuffers(fxaaFBO_, 1, &buf);

	if (glCheckNamedFramebufferStatus(fxaaFBO_, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "FXAA FBO incomplete\n";
	}
} // end of resize()

void FXAAPass::destroyGL()
{
	if (fsVao_)
	{
		glDeleteVertexArrays(1, &fsVao_);
		fsVao_ = 0;
	}
} // end of destroyGL()

void FXAAPass::render(uint32_t sceneColorTex, int w, int h)
{
	if (!enabled_ || !shader_ || !sceneColorTex || w <= 0 || h <= 0)
		return;

	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, fxaaFBO_);
	glBindVertexArray(fsVao_);

	glViewport(0, 0, w, h);
	glClear(GL_COLOR_BUFFER_BIT);

	shader_->use();
	shader_->setInt("u_sceneColorTex", 0);
	shader_->setVec2("u_inverseScreenSize", glm::vec2(1.0f / static_cast<float>(w), 1.0f / static_cast<float>(h)));

	shader_->setFloat("u_edgeSharpnessQuality", edgeSharpnessQuality_);
	shader_->setFloat("u_edgeThresholdMax", edgeThresholdMax_);
	shader_->setFloat("u_edgeThresholdMin", edgeThresholdMin_);

	glBindTextureUnit(0, sceneColorTex);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
} // end of render()

void FXAAPass::setEnabled(bool e) 
{ 
	enabled_ = e; 
} // end of setEnabled()

bool FXAAPass::enabled() const 
{ 
	return enabled_; 
} // end of enabled()

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
