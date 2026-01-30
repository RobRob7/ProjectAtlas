#include "fogpass.h"

//--- PUBLIC ---//
FogPass::~FogPass()
{
	destroyGL();
} // end of destructor

void FogPass::init()
{
	shader_.emplace("fogpass/fog.vert", "fogpass/fog.frag");

	glCreateVertexArrays(1, &fsVao_);
} // end of init()

void FogPass::destroyGL()
{
	if (fsVao_)
	{
		glDeleteVertexArrays(1, &fsVao_);
		fsVao_ = 0;
	}
} // end of destroyGL()

void FogPass::render(uint32_t sceneColorTex, uint32_t sceneDepthTex, int w, int h, 
	float nearPlane, float farPlane, float ambStr)
{
	if (!enabled_ || !shader_ || !sceneColorTex || !sceneDepthTex || w <= 0 || h <= 0)
		return;

	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(fsVao_);

	shader_->use();
	shader_->setInt("u_sceneColorTex", 0);
	shader_->setInt("u_sceneDepthTex", 1);

	shader_->setFloat("u_near", nearPlane);
	shader_->setFloat("u_far", farPlane);

	shader_->setVec3("u_fogColor", fogColor_);
	shader_->setFloat("u_fogStart", fogStart_);
	shader_->setFloat("u_fogEnd", fogEnd_);

	shader_->setFloat("u_ambStr", ambStr);

	glBindTextureUnit(0, sceneColorTex);
	glBindTextureUnit(1, sceneDepthTex);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
} // end of render()

void FogPass::setEnabled(bool e) 
{ 
	enabled_ = e; 
} // end of setEnabled()

bool FogPass::enabled() const 
{ 
	return enabled_; 
} // end of enabled()

void FogPass::setFogColor(glm::vec3 v)
{
	fogColor_ = v;
} // end of setFogColor()

void FogPass::setFogStart(float v)
{
	fogStart_ = v;
} // end of setFogStart()

void FogPass::setFogEnd(float v)
{
	fogEnd_ = v;
} // end of setFogEnd()
