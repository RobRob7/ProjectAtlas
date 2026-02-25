#include "fog_pass.h"

#include "shader.h"

#include <glad/glad.h>

//--- PUBLIC ---//
FogPass::FogPass() = default;

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

	ubo_.init<sizeof(FogPassUBO)>();

	glCreateVertexArrays(1, &fsVao_);
} // end of init()

void FogPass::render(uint32_t sceneColorTex, uint32_t sceneDepthTex, 
	float nearPlane, float farPlane, float ambStr)
{
	if (!shader_ || !sceneColorTex || !sceneDepthTex || fsVao_ == 0)
		return;

	const GLboolean prevDepth = glIsEnabled(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(fsVao_);
	
	shader_->use();
	fogPassUBO_.u_near = nearPlane;
	fogPassUBO_.u_far = farPlane;
	fogPassUBO_.u_fogColor = fogColor_;
	fogPassUBO_.u_fogStart = fogStart_;
	fogPassUBO_.u_fogEnd = fogEnd_;
	fogPassUBO_.u_ambStr = ambStr;
	ubo_.update(&fogPassUBO_, sizeof(fogPassUBO_));

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