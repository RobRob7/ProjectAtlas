#include "cubemap.h"

//--- PUBLIC ---//
CubeMap::CubeMap(const std::vector<std::string>& textures)
	: shader_("/cubemap/cubemap.vert", "/cubemap/cubemap.frag"), texture_(textures), cubemapTexture_(texture_.m_ID)
{
	// VAO + VBO
	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);
	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SkyboxVertices), &SkyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	shader_.use();
	shader_.setInt("skybox", 0);
} // end of constructor

// destructor
CubeMap::~CubeMap()
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

// render cubemap
void CubeMap::render(glm::mat4& view, glm::mat4& projection, const float time) const
{
	// remove translation from camera view
	glm::mat4 viewStrippedTranslation = glm::mat4(glm::mat3(view));

	if (time > 0.0f)
	{
		float speed = 0.005f;

		glm::mat4 skyRot = glm::rotate(glm::mat4(1.0f),
			time * speed,
			glm::vec3(1.0f, 1.0f, 0.0f));
		viewStrippedTranslation = glm::mat4(glm::mat3(skyRot)) * viewStrippedTranslation;
	}

	glDepthFunc(GL_LEQUAL);
	shader_.use();
	shader_.setMat4("u_view", viewStrippedTranslation);
	shader_.setMat4("u_projection", projection);

	glBindVertexArray(vao_);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture_);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS);
} // end of render()