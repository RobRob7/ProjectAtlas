#include "fogpass.h"

#include "shader.h"

#include <glad/glad.h>

//--- PUBLIC ---//
FogPass::~FogPass()
{
	destroyGL();
} // end of destructor

void FogPass::init()
{
	destroyGL();

	shader_ = std::make_unique<Shader>("fogpass/fog.vert", "fogpass/fog.frag");

	shader_->use();
	shader_->setInt("u_sceneColorTex", 0);
	shader_->setInt("u_sceneDepthTex", 1);

	glCreateVertexArrays(1, &fsVao_);
} // end of init()

void FogPass::render(uint32_t sceneColorTex, uint32_t sceneDepthTex, 
	float nearPlane, float farPlane, float ambStr)
{
	if (!shader_ || !sceneColorTex || !sceneDepthTex)
		return;

	const GLboolean prevDepth = glIsEnabled(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(fsVao_);
	
	shader_->use();
	shader_->setFloat("u_near", nearPlane);
	shader_->setFloat("u_far", farPlane);

	shader_->setVec3("u_fogColor", fogColor_);
	shader_->setFloat("u_fogStart", fogStart_);
	shader_->setFloat("u_fogEnd", fogEnd_);

	shader_->setFloat("u_ambStr", ambStr);

	glBindTextureUnit(0, sceneColorTex);
	glBindTextureUnit(1, sceneDepthTex);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	if (prevDepth) glEnable(GL_DEPTH_TEST);
} // end of render()

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


//--- PRIVATE ---//
void FogPass::destroyGL()
{
	if (fsVao_)
	{
		glDeleteVertexArrays(1, &fsVao_);
		fsVao_ = 0;
	}
} // end of destroyGL()