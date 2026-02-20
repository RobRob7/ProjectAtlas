#include "debug_pass.h"

#include "shader.h"

#include <glad/glad.h>

//--- PUBLIC ---//
DebugPass::DebugPass() = default;

DebugPass::~DebugPass()
{
	destroyGL();
} // end of destructor

void DebugPass::init()
{
	destroyGL();

	debugShader_ = std::make_unique<Shader>("debugpass/debugpass.vert", "debugpass/debugpass.frag");

	glCreateVertexArrays(1, &vao_);

	debugShader_->use();
	debugShader_->setInt("u_normal", 0);
	debugShader_->setInt("u_depth", 1);
} // end of init()

void DebugPass::destroyGL()
{
	if (vao_)
	{
		glDeleteVertexArrays(1, &vao_);
		vao_ = 0;
	}
} // end of destroyGL()

void DebugPass::render(uint32_t normalTex, uint32_t depthTex, float nearPlane, float farPlane, int mode)
{
	if (!debugShader_ || vao_ == 0) return;

	const GLboolean prevDepth = glIsEnabled(GL_DEPTH_TEST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);

	debugShader_->use();
	debugShader_->setInt("u_mode", mode);
	debugShader_->setFloat("u_near", nearPlane);
	debugShader_->setFloat("u_far", farPlane);

	glBindTextureUnit(0, normalTex);
	glBindTextureUnit(1, depthTex);

	glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	if (prevDepth) glEnable(GL_DEPTH_TEST);
} // end of render()