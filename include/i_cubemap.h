#ifndef I_CUBEMAP_H
#define I_CUBEMAP_H

#include "constants.h"

#include <glm/glm.hpp>

struct FrameContext;

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
	virtual void render(
		const FrameContext* frame,
		const glm::mat4& view, 
		const glm::mat4& projection, 
		const float time = -1.0
	) = 0;
	virtual void renderOffscreen(
		const FrameContext* frame,
		const glm::mat4& view,
		const glm::mat4& projection,
		const float time = -1.0
	) 
	{
	};
};

#endif
