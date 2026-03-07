#ifndef I_CUBEMAP_H
#define I_CUBEMAP_H

#include "constants.h"

#include <glm/glm.hpp>

class ICubemap
{
public:
	ICubemap() = default;
	virtual ~ICubemap() = default;

	ICubemap(const ICubemap&) = delete;
	ICubemap& operator=(const ICubemap&) = delete;

	ICubemap(ICubemap&&) = default;
	ICubemap& operator=(ICubemap&&) = default;

	virtual void init() = 0;
	virtual void render(const RenderContext& ctx, const glm::mat4& view, const glm::mat4& projection, const float time = -1.0) = 0;
};

#endif
