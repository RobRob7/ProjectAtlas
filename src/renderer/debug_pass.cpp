#include "debug_pass.h"

#include "texture_bindings.h"

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

	ubo_.init<sizeof(DebugPassUBO)>();

	glCreateVertexArrays(1, &vao_);
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
	debugPassUBO_.u_mode = mode;
	debugPassUBO_.u_near = nearPlane;
	debugPassUBO_.u_far = farPlane;
	ubo_.update(&debugPassUBO_, sizeof(debugPassUBO_));

	glBindTextureUnit(TO_API_FORM(TextureBinding::GNormalTex), normalTex);
	glBindTextureUnit(TO_API_FORM(TextureBinding::GDepthTex), depthTex);

	glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	if (prevDepth) glEnable(GL_DEPTH_TEST);
} // end of render()