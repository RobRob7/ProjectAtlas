#ifndef RENDERER_H
#define RENDERER_H

#include "i_renderer.h"
#include "render_settings.h"

#include <cstdint>
#include <memory>

class ChunkOpaquePassGL;

class ChunkManager;
class Camera;
class Light;
class CubeMap;
class Crosshair;

class GBufferPass;
class DebugPass;
class SSAOPass;
class FXAAPass;
class PresentPass;
class WaterPass;
class FogPass;

struct RenderInputs;

class Renderer : public IRenderer
{
public:
	Renderer();
	~Renderer() override;

	void init() override;
	void resize(int w, int h) override;
	void renderFrame(const RenderInputs& in) override;

	RenderSettings& settings() override;

private:
	void destroyGL();
	void resizeForwardTargets();
private:
	int width_{};
	int height_{};

	std::unique_ptr<RenderSettings> renderSettings_;

	// passes
	std::unique_ptr<GBufferPass> gbuffer_;
	std::unique_ptr<DebugPass> debugPass_;
	std::unique_ptr<SSAOPass> ssaoPass_;
	std::unique_ptr<FXAAPass> fxaaPass_;
	std::unique_ptr<FogPass> fogPass_;
	std::unique_ptr<PresentPass> presentPass_;
	std::unique_ptr<WaterPass> waterPass_;

	uint32_t forwardFBO_{};
	uint32_t forwardColorTex_{};
	uint32_t forwardDepthTex_{};

	std::unique_ptr<ChunkOpaquePassGL> chunkOpaque_;
};

#endif
