#ifndef RENDERER_H
#define RENDERER_H

#include "chunkmanager.h"
#include "camera.h"
#include "light.h"
#include "cubemap.h"
#include "crosshair.h"

#include "gbufferpass.h"
#include "debugpass.h"
#include "ssaopass.h"
#include "fxaapass.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <cstdint>
#include <iostream>

enum class DebugMode : int
{
	None	= 0, // '1' key
	Normals = 1, // '2' key
	Depth	= 2, // '3' key
};

struct RenderSettings
{
	// debug view mode
	DebugMode debugMode = DebugMode::None;

	// display options
	bool enableVsync = true;

	// graphics options
	bool useSSAO = false;
	bool useFXAA = false;
};

struct RenderInputs
{
	ChunkManager* world = nullptr;
	Camera* camera = nullptr;
	Light* light = nullptr;
	CubeMap* skybox = nullptr;
	Crosshair* crosshair = nullptr;

	float time = 0.0f;
};

class Renderer
{
public:
	void init();
	void resize(int w, int h);
	void renderFrame(const RenderInputs& in);

	RenderSettings& settings();

private:
	void fxaaResize();

	void waterResize();
	void waterPass(const RenderInputs& in);
	void waterReflectionPass(const RenderInputs& in);
	void waterRefractionPass(const RenderInputs& in);
private:
	int width_{};
	int height_{};

	RenderSettings renderSettings_;

	// passes
	GBufferPass gbuffer_;
	DebugPass debugPass_;
	SSAOPass ssaoPass_;
	FXAAPass fxaaPass_;

	uint32_t forwardFBO_{};
	uint32_t forwardColorTex_{};
	uint32_t forwardDepthRBO_{};

	uint32_t reflFBO_{};
	uint32_t reflColorTex_{};
	uint32_t reflDepthRBO_{};

	uint32_t refrFBO_{};
	uint32_t refrColorTex_{};
	uint32_t refrDepthTex_{};
};

#endif
