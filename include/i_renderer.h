#ifndef IRENDERER_H
#define IRENDERER_H

#include "render_inputs.h"

class IRenderer
{
public:
	virtual ~IRenderer() = default;
	virtual void init() = 0;
	virtual void resize(int w, int h) = 0;
	virtual void renderFrame(const RenderInputs& in) = 0;
};

#endif
