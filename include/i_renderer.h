#ifndef I_RENDERER_H
#define I_RENDERER_H

struct RenderInputs;
struct RenderSettings;
struct FrameContext;
class UIVk;

class IRenderer
{
public:
	virtual ~IRenderer() = default;

	virtual void init() = 0;
	virtual void resize(int w, int h) = 0;

	virtual void renderFrame(
		const RenderInputs& in, 
		const FrameContext* frame, 
		UIVk* ui
	) = 0;
	
	virtual RenderSettings& settings() = 0;
};

#endif
