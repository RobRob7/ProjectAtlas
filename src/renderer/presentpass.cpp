#include "presentpass.h"

//--- PUBLIC ---//
PresentPass::~PresentPass()
{
	destroyGL();
} // end of destructor

void PresentPass::init()
{
	shader_.emplace("presentpass/present.vert", "presentpass/present.frag");

	glCreateVertexArrays(1, &fsVao_);
} // end of init()

void PresentPass::destroyGL()
{
	if (fsVao_)
	{
		glDeleteVertexArrays(1, &fsVao_);
		fsVao_ = 0;
	}
} // end of destroyGL()

void PresentPass::render(uint32_t sceneColorTex, int w, int h)
{
	if (!shader_ || !sceneColorTex || w <= 0 || h <= 0) return;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDisable(GL_DEPTH_TEST);

	shader_->use();
	shader_->setInt("u_sceneColorTex", 0);

	glBindTextureUnit(0, sceneColorTex);

	glBindVertexArray(fsVao_);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glBindVertexArray(0);
} // end of render()