#include "light.h"

//--- PUBLIC ---//
Light::Light(const glm::vec3& pos, const glm::vec3& color)
	: position_(pos), color_(color)
{
} // end of constructor

Light::~Light()
{
	if (vao_)
	{
		glDeleteVertexArrays(1, &vao_);
		vao_ = 0;
	}
	if (vbo_)
	{
		glDeleteBuffers(1, &vbo_);
		vbo_ = 0;
	}
} // end of destructor

void Light::init()
{
	shader_.emplace("light/light.vert", "light/light.frag");

	// vao + vbo
	glCreateVertexArrays(1, &vao_);
	glCreateBuffers(1, &vbo_);

	// upload data to buffer
	glNamedBufferData(vbo_, sizeof(CUBE_VERTICES), CUBE_VERTICES.data(), GL_STATIC_DRAW);

	// attach buffers to vao
	glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, sizeof(Vertex));

	// position
	glEnableVertexArrayAttrib(vao_, 0);
	glVertexArrayAttribFormat(vao_, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, pos));
	glVertexArrayAttribBinding(vao_, 0, 0);

	// normal
	glEnableVertexArrayAttrib(vao_, 1);
	glVertexArrayAttribFormat(vao_, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
	glVertexArrayAttribBinding(vao_, 1, 0);

	// uv
	glEnableVertexArrayAttrib(vao_, 2);
	glVertexArrayAttribFormat(vao_, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
	glVertexArrayAttribBinding(vao_, 2, 0);
} // end of init()

void Light::render(const glm::mat4& view, const glm::mat4& proj)
{
	shader_->use();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position_);

	shader_->setVec3("u_color", color_);
	shader_->setMat4("u_model", model);
	shader_->setMat4("u_view", view);
	shader_->setMat4("u_proj", proj);

	glBindVertexArray(vao_);
	glDrawArrays(GL_TRIANGLES, 0, 36);
} // end of render

glm::vec3& Light::getPosition()
{
	return position_;
} // end of getPosition()

const glm::vec3& Light::getPosition() const
{
	return position_;
} // end of getPosition()

glm::vec3& Light::getColor()
{
	return color_;
} // end of getColor()

const glm::vec3& Light::getColor() const
{
	return color_;
} // end of getColor()

void Light::setPosition(const glm::vec3& pos)
{
	position_ = pos;
} // end of setPosition()

void Light::setColor(const glm::vec3& color)
{
	color_ = {
		std::clamp(color.x, MIN_COLOR, MAX_COLOR),
		std::clamp(color.y, MIN_COLOR, MAX_COLOR),
		std::clamp(color.z, MIN_COLOR, MAX_COLOR)
	};
} // end of setColor()