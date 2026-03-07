#ifndef I_LIGHT_H
#define I_LIGHT_H

#include "constants.h"

#include <glm/glm.hpp>

class ILight
{
public:
	virtual ~ILight() = default;

	virtual void init() = 0;
	virtual void render(const RenderContext& ctx, const glm::mat4& view, const glm::mat4& proj) = 0;

	virtual glm::vec3& getPosition() = 0;
	virtual const glm::vec3& getPosition() const = 0;
	virtual glm::vec3& getColor() = 0;
	virtual const glm::vec3& getColor() const = 0;

	virtual void setPosition(const glm::vec3& pos) = 0;
	virtual void setColor(const glm::vec3& color) = 0;
};

#endif
