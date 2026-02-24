#ifndef LIGHT_GL_H
#define LIGHT_GL_H

#include "i_light.h"

#include "ubo_gl.h"

#include <cstdint>
#include <memory>

class Shader;

using namespace Light_Constants;

class LightGL final : public ILight
{
public:
	LightGL(const glm::vec3& pos, const glm::vec3& color = glm::vec3(1.0f));
	~LightGL() override;

	void init() override;
	void render(const glm::mat4& view, const glm::mat4& proj) override;

	glm::vec3& getPosition() override;
	const glm::vec3& getPosition() const override;
	glm::vec3& getColor() override;
	const glm::vec3& getColor() const override;

	void setPosition(const glm::vec3& pos) override;
	void setColor(const glm::vec3& color) override;

private:
	void destroyGL();
private:
	std::unique_ptr<Shader> shader_;
	uint32_t vao_{};
	uint32_t vbo_{};
	UBOGL ubo_{UBOBinding::Light};

	glm::vec3 position_{};
	glm::vec3 color_{};
};

#endif
