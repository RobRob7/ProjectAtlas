#ifndef LIGHT_H
#define LIGHT_H

#include "shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

#include <array>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <algorithm>

struct VertexLight
{
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 uv;
};

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

class Light
{
public:
	const float MIN_COLOR = 0.0f;
	const float MAX_COLOR = 1.0f;
public:
	Light(const glm::vec3& pos, const glm::vec3& color = glm::vec3(1.0f));
	~Light();

	void init();
	void render(const glm::mat4& view, const glm::mat4& proj);

	glm::vec3& getPosition();
	const glm::vec3& getPosition() const;
	glm::vec3& getColor();
	const glm::vec3& getColor() const;

	void setPosition(const glm::vec3& pos);
	void setColor(const glm::vec3& color);

private:
	std::optional<Shader> shader_;
	uint32_t vao_{};
	uint32_t vbo_{};

	glm::vec3 position_{};
	glm::vec3 color_{};
};

#endif
