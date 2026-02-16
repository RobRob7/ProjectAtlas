#include "cubemap.h"

#include "texture.h"
#include "shader.h"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

const std::array<float, 108> SkyboxVertices =
{
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

// cubemap default
const std::array<std::string_view, 6> DEFAULT_FACES = { {
	"texture/cubemap/space_alt/right.png",
	"texture/cubemap/space_alt/left.png",
	"texture/cubemap/space_alt/top.png",
	"texture/cubemap/space_alt/bottom.png",
	"texture/cubemap/space_alt/front.png",
	"texture/cubemap/space_alt/back.png"
} };

//--- PUBLIC ---//
CubeMap::CubeMap(const std::array<std::string_view, 6>& textures)
	: faces_(textures)
{
} // end of constructor

// destructor
CubeMap::~CubeMap()
{
	destroyGL();
} // end of destructor

void CubeMap::init()
{
	destroyGL();

	shader_ = std::make_unique<Shader>("cubemap/cubemap.vert", "cubemap/cubemap.frag");
	texture_ = std::make_unique<Texture>(faces_);

	// VAO + VBO
	glCreateVertexArrays(1, &vao_);
	glCreateBuffers(1, &vbo_);

	// upload vertex data
	glNamedBufferData(vbo_, SkyboxVertices.size() * sizeof(float), SkyboxVertices.data(), GL_STATIC_DRAW);

	// attach vbo to vao
	glVertexArrayVertexBuffer(vao_, 0, vbo_, 0, 3 * sizeof(float));

	// pos attribute = 0
	glEnableVertexArrayAttrib(vao_, 0);
	glVertexArrayAttribFormat(vao_, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao_, 0, 0);

	shader_->use();
	shader_->setInt("skybox", 0);
} // end of init()

// render cubemap
void CubeMap::render(const glm::mat4& view, const glm::mat4& projection, const float time) const
{
	if (!shader_ || !texture_ || vao_ == 0)
		return;

	// remove translation from camera view
	glm::mat4 viewStrippedTranslation = glm::mat4(glm::mat3(view));

	if (time > 0.0f)
	{
		float speed = 0.005f;

		glm::mat4 skyRot = glm::rotate(glm::mat4(1.0f),
			time * speed,
			glm::vec3(0.0f, 1.0f, 0.0f));
		viewStrippedTranslation = glm::mat4(glm::mat3(skyRot)) * viewStrippedTranslation;
	}

	GLint prevFunc;
	glGetIntegerv(GL_DEPTH_FUNC, &prevFunc);
	GLboolean prevMask;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &prevMask);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_FALSE);

	shader_->use();
	shader_->setMat4("u_view", viewStrippedTranslation);
	shader_->setMat4("u_projection", projection);

	glBindVertexArray(vao_);
	glBindTextureUnit(0, texture_->ID());

	glDrawArrays(GL_TRIANGLES, 0, 36);

	//glDepthMask(GL_TRUE);
	//glDepthFunc(GL_LESS);
	glDepthMask(prevMask);
	glDepthFunc(prevFunc);
} // end of render()


//--- PRIVATE ---//
void CubeMap::destroyGL()
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
} // end of destroyGL()