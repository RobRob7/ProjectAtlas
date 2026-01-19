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
	glBindVertexArray(fsVao_);

	shader_->use();
	shader_->setInt("u_sceneColorTex", 0);
	shader_->setVec2("u_inverseScreenSize", glm::vec2(1.0f / static_cast<float>(w), 1.0f / static_cast<float>(h)));

	shader_->setFloat("u_edgeSharpnessQuality", edgeSharpnessQuality_);
	shader_->setFloat("u_edgeThresholdMax", edgeThresholdMax_);
	shader_->setFloat("u_edgeThresholdMin", edgeThresholdMin_);

	glBindTextureUnit(0, sceneColorTex);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
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
