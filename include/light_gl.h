#ifndef LIGHT_GL_H
#define LIGHT_GL_H

#include "constants.h"
#include "bindings.h"

#include "i_light.h"

#include "ubo_gl.h"

#include <cstdint>
#include <memory>
#include <algorithm>

class Shader;

class LightGL final : public ILight
{
public:
	LightGL(
		const glm::vec3& pos,
		const glm::vec3& dir = glm::vec3(-0.6f, -1.0f, -0.35f),
		const glm::vec3& color = glm::vec3(1.0f)
	);
	~LightGL() override;

	void init() override;

	void render(
		const FrameContext* frame,
		const glm::mat4& view,
		const glm::mat4& proj
	) override;

	void updateLightDirection(float time) override;

	float& getSpeed() override { return speed_; }
	const float& getSpeed() const override { return speed_; }
	glm::vec3& getDirection() override { return direction_; }
	const glm::vec3& getDirection() const override { return direction_; }
	glm::vec3& getPosition() override { return position_; }
	const glm::vec3& getPosition() const override { return position_; }
	glm::vec3& getColor() override { return color_; }
	const glm::vec3& getColor() const override { return color_; }

	void setSpeed(const float speed) override
	{
		speed_ = std::clamp(speed, MIN_SPEED, MAX_SPEED);
	} // end of setSpeed()

	void setDirection(const glm::vec3& dir) override
	{
		if (glm::length(dir) > 0.0001f)
			direction_ = glm::normalize(dir);
	} // end of setDirection()

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

	UBOGL ubo_{ TO_API_FORM(LightBinding::UBO) };

	float speed_{ 0.1f };
	glm::vec3 direction_{};
	glm::vec3 position_{};
	glm::vec3 color_{};
};

#endif
