#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

#include <array>
#include <cstdint>
#include <memory>

class Shader;

extern const std::array<float, 288> CUBE_VERTICES;

class Light
{
public:
	static constexpr float MIN_COLOR = 0.0f;
    static constexpr float MAX_COLOR = 1.0f;
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
	void destroyGL();
private:
	std::unique_ptr<Shader> shader_;
	uint32_t vao_{};
	uint32_t vbo_{};

	glm::vec3 position_{};
	glm::vec3 color_{};
};

#endif
