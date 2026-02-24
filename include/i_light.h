#ifndef I_LIGHT_H
#define I_LIGHT_H

#include <glm/glm.hpp>

#include <array>

namespace Light_Constants
{
	inline constexpr float MIN_COLOR = 0.0f;
	inline constexpr float MAX_COLOR = 1.0f;

	// pos, normals, texcoords
	inline constexpr std::array<float, 288> CUBE_VERTICES = {
		// =========================
		// Back face (Z-)
		// =========================
		-0.5f,-0.5f,-0.5f,   0.0f,0.0f,-1.0f,   0.0f,0.0f,
		 0.5f,-0.5f,-0.5f,   0.0f,0.0f,-1.0f,   1.0f,0.0f,
		 0.5f, 0.5f,-0.5f,   0.0f,0.0f,-1.0f,   1.0f,1.0f,
		 0.5f, 0.5f,-0.5f,   0.0f,0.0f,-1.0f,   1.0f,1.0f,
		-0.5f, 0.5f,-0.5f,   0.0f,0.0f,-1.0f,   0.0f,1.0f,
		-0.5f,-0.5f,-0.5f,   0.0f,0.0f,-1.0f,   0.0f,0.0f,

		// =========================
		// Front face (Z+)
		// =========================
		-0.5f,-0.5f, 0.5f,   0.0f,0.0f, 1.0f,   0.0f,0.0f,
		 0.5f,-0.5f, 0.5f,   0.0f,0.0f, 1.0f,   1.0f,0.0f,
		 0.5f, 0.5f, 0.5f,   0.0f,0.0f, 1.0f,   1.0f,1.0f,
		 0.5f, 0.5f, 0.5f,   0.0f,0.0f, 1.0f,   1.0f,1.0f,
		-0.5f, 0.5f, 0.5f,   0.0f,0.0f, 1.0f,   0.0f,1.0f,
		-0.5f,-0.5f, 0.5f,   0.0f,0.0f, 1.0f,   0.0f,0.0f,

		// =========================
		// Left face (X-)
		// =========================
		-0.5f, 0.5f, 0.5f,  -1.0f,0.0f,0.0f,   1.0f,0.0f,
		-0.5f, 0.5f,-0.5f,  -1.0f,0.0f,0.0f,   1.0f,1.0f,
		-0.5f,-0.5f,-0.5f,  -1.0f,0.0f,0.0f,   0.0f,1.0f,
		-0.5f,-0.5f,-0.5f,  -1.0f,0.0f,0.0f,   0.0f,1.0f,
		-0.5f,-0.5f, 0.5f,  -1.0f,0.0f,0.0f,   0.0f,0.0f,
		-0.5f, 0.5f, 0.5f,  -1.0f,0.0f,0.0f,   1.0f,0.0f,

		// =========================
		// Right face (X+)
		// =========================
		 0.5f, 0.5f, 0.5f,   1.0f,0.0f,0.0f,   1.0f,0.0f,
		 0.5f, 0.5f,-0.5f,   1.0f,0.0f,0.0f,   1.0f,1.0f,
		 0.5f,-0.5f,-0.5f,   1.0f,0.0f,0.0f,   0.0f,1.0f,
		 0.5f,-0.5f,-0.5f,   1.0f,0.0f,0.0f,   0.0f,1.0f,
		 0.5f,-0.5f, 0.5f,   1.0f,0.0f,0.0f,   0.0f,0.0f,
		 0.5f, 0.5f, 0.5f,   1.0f,0.0f,0.0f,   1.0f,0.0f,

		 // =========================
		 // Bottom face (Y-)
		 // =========================
		 -0.5f,-0.5f,-0.5f,   0.0f,-1.0f,0.0f,   0.0f,1.0f,
		  0.5f,-0.5f,-0.5f,   0.0f,-1.0f,0.0f,   1.0f,1.0f,
		  0.5f,-0.5f, 0.5f,   0.0f,-1.0f,0.0f,   1.0f,0.0f,
		  0.5f,-0.5f, 0.5f,   0.0f,-1.0f,0.0f,   1.0f,0.0f,
		 -0.5f,-0.5f, 0.5f,   0.0f,-1.0f,0.0f,   0.0f,0.0f,
		 -0.5f,-0.5f,-0.5f,   0.0f,-1.0f,0.0f,   0.0f,1.0f,

		 // =========================
		 // Top face (Y+)
		 // =========================
		 -0.5f, 0.5f,-0.5f,   0.0f,1.0f,0.0f,   0.0f,1.0f,
		  0.5f, 0.5f,-0.5f,   0.0f,1.0f,0.0f,   1.0f,1.0f,
		  0.5f, 0.5f, 0.5f,   0.0f,1.0f,0.0f,   1.0f,0.0f,
		  0.5f, 0.5f, 0.5f,   0.0f,1.0f,0.0f,   1.0f,0.0f,
		 -0.5f, 0.5f, 0.5f,   0.0f,1.0f,0.0f,   0.0f,0.0f,
		 -0.5f, 0.5f,-0.5f,   0.0f,1.0f,0.0f,   0.0f,1.0f
	};

	struct VertexLight
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
	};
};

class ILight
{
public:
	virtual ~ILight() = default;

	virtual void init() = 0;
	virtual void render(const glm::mat4& view, const glm::mat4& proj) = 0;

	virtual glm::vec3& getPosition() = 0;
	virtual const glm::vec3& getPosition() const = 0;
	virtual glm::vec3& getColor() = 0;
	virtual const glm::vec3& getColor() const = 0;

	virtual void setPosition(const glm::vec3& pos) = 0;
	virtual void setColor(const glm::vec3& color) = 0;
};

#endif
