#ifndef I_LIGHT_H
#define I_LIGHT_H

#include "constants.h"

#include <glm/glm.hpp>

struct FrameContext;

class ILight
{
public:
	virtual ~ILight() = default;

	virtual void init() = 0;

	virtual void render(
		const FrameContext* frame,
		const glm::mat4& view, 
		const glm::mat4& proj
	) = 0;
	virtual void renderOffscreen(
		const FrameContext* frame,
		const glm::mat4& view,
		const glm::mat4& proj,
		const glm::vec3& position,
		uint32_t width,
		uint32_t height
	)
	{
	}

	virtual glm::vec3& getPosition() = 0;
	virtual const glm::vec3& getPosition() const = 0;
	virtual glm::vec3& getColor() = 0;
	virtual const glm::vec3& getColor() const = 0;

	virtual void setPosition(const glm::vec3& pos) = 0;
	virtual void setColor(const glm::vec3& color) = 0;
};

#endif
