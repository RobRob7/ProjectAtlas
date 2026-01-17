#include "fxaapass.h"

//--- PUBLIC ---//
FXAAPass::~FXAAPass()
{
	destroyGL();
} // end of destructor

void FXAAPass::init()
{
	shader_.emplace("fxaapass/fxaa.vert", "fxaapass/fxaaMOD.frag");

	glCreateVertexArrays(1, &fsVao_);
} // end of init()

void FXAAPass::resize(float w, float h)
{

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
	glBindVertexArray(fsVao_);

	shader_->use();
	shader_->setInt("u_sceneColorTex", 0);
	shader_->setVec2("u_inverseScreenSize", glm::vec2(1.0f / (float)w, 1.0f / (float)h));

	shader_->setInt("u_maxIterations", maxIterations_);
	shader_->setFloat("u_subPixelQuality", subPixelQuality_);
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

void FXAAPass::setSubpix(float v) 
{ 
	subPixelQuality_ = v;
} // end of setSubpix()

void FXAAPass::setEdgeThreshold(float v) 
{ 
	edgeThresholdMax_ = v;
} // end of setEdgeThreshold()

void FXAAPass::setEdgeThresholdMin(float v) 
{
	edgeThresholdMin_ = v; 
} // end of setEdgeThresholdMin()