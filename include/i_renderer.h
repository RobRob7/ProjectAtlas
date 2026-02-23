#ifndef I_RENDERER_H
#define I_RENDERER_H

#include "render_inputs.h"

struct RenderSettings;

class IRenderer
{
public:
	virtual ~IRenderer() = default;
	virtual void init() = 0;
	virtual void resize(int w, int h) = 0;
	virtual void renderFrame(const RenderInputs& in) = 0;
	virtual RenderSettings& settings() = 0;
};

#endif
