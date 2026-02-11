#include "crosshair.h"

#include "shader.h"

#include <glad/glad.h>

//--- PUBLIC ---//
Crosshair::Crosshair(float size)
	: size_(size)
{
} // end of constructor

Crosshair::~Crosshair()
{
	destroyGL();
} // end of destructor

void Crosshair::init()
{
	crosshairShader_ = std::make_unique<Shader>("crosshair/crosshair.vert", "crosshair/crosshair.frag");

	glm::vec2 center{ 0.0f, 0.0f };

	float vertices[] = {
		// Horizontal line (x from -size_ to +size_ at y = 0)
		center.x - size_, center.y,
		center.x + size_, center.y,

		// Vertical line (y from -size_ to +size_ at x = 0)
		center.x, center.y - size_,
		center.x, center.y + size_
	};

	glCreateVertexArrays(1, &vao_);
	glCreateBuffers(1, &vbo_);

	glNamedBufferData(vbo_, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// attach buffer to vao
	glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(float) * 2);

	glEnableVertexArrayAttrib(vao_, 0);
	glVertexArrayAttribFormat(vao_, 0, 2, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao_, 0, 0);
} // end of init()

void Crosshair::render()
{
	// Disable depth so crosshair draws on top
	glDisable(GL_DEPTH_TEST);

	crosshairShader_->use();
	glBindVertexArray(vao_);
	glDrawArrays(GL_LINES, 0, 4);

	// re-enable depth test after drawing
	glEnable(GL_DEPTH_TEST);
} // end of render()


//--- PRIVATE ---//
void Crosshair::destroyGL()
{
	if (vao_)
	{
		glDeleteVertexArrays(1, &vao_);
	}
	if (vbo_)
	{
		glDeleteBuffers(1, &vbo_);
	}
} // end of destroyGL()