#include "crosshair.h"

//--- PUBLIC ---//
Crosshair::Crosshair(const float size)
	: size_(size)
{
	crosshairShader_ = Shader("crosshair/crosshair.vert", "crosshair/crosshair.frag");
} // end of constructor

Crosshair::~Crosshair()
{
	if (vao_)
	{
		glDeleteVertexArrays(1, &vao_);
	}
	if (vbo_)
	{
		glDeleteBuffers(1, &vbo_);
	}
} // end of destructor

void Crosshair::render()
{
	// Disable depth so crosshair draws on top
	glDisable(GL_DEPTH_TEST);

	glm::vec2 center{ 0.0f, 0.0f };

	float vertices[] = {
		// Horizontal line (x from -size_ to +size_ at y = 0)
		center.x - size_, center.y,
		center.x + size_, center.y,

		// Vertical line (y from -size_ to +size_ at x = 0)
		center.x, center.y - size_,
		center.x, center.y + size_
	};

	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);
	glBindVertexArray(vao_);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	crosshairShader_.use();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_LINES, 0, 4);

	glDeleteBuffers(1, &vbo_);
	glDeleteVertexArrays(1, &vao_);

	// re-enable depth test after drawing
	glEnable(GL_DEPTH_TEST);
} // end of render()