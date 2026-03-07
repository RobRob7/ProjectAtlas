#ifndef LIGHT_GL_H
#define LIGHT_GL_H

#include "i_light.h"

#include "ubo_gl.h"
#include "ubo_bindings.h"

#include <cstdint>
#include <memory>
#include <algorithm>

class Shader;

class LightGL final : public ILight
{
public:
	LightGL(const glm::vec3& pos, const glm::vec3& color = glm::vec3(1.0f));
	~LightGL() override;

	void init() override;
	void render(const RenderContext& ctx, const glm::mat4& view, const glm::mat4& proj) override;

	glm::vec3& getPosition() override { return position_; }
	const glm::vec3& getPosition() const override { return position_; }
	glm::vec3& getColor() override { return color_; }
	const glm::vec3& getColor() const override { return color_; }

	void setPosition(const glm::vec3& pos) override { position_ = pos; }

	void setColor(const glm::vec3& color) override
	{
		color_ = {
		std::clamp(color.x, Light_Constants::MIN_COLOR, Light_Constants::MAX_COLOR),
		std::clamp(color.y, Light_Constants::MIN_COLOR, Light_Constants::MAX_COLOR),
		std::clamp(color.z, Light_Constants::MIN_COLOR, Light_Constants::MAX_COLOR)
		};
	} // end of setColor()

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
