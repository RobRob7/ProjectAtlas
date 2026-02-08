#ifndef RENDERER_H
#define RENDERER_H

#include "renderinputs.h"

#include "chunkmanager.h"
#include "camera.h"
#include "light.h"
#include "cubemap.h"
#include "crosshair.h"

#include "gbufferpass.h"
#include "debugpass.h"
#include "ssaopass.h"
#include "fxaapass.h"
#include "presentpass.h"
#include "waterpass.h"

#include "fogpass.h"

#include <glm/glm.hpp>
#include <glad/glad.h>

#include <cstdint>
#include <iostream>
#include <stdexcept>

enum class DebugMode : int
{
	None	= 0, // '1' key
	Normals = 1, // '2' key
	Depth	= 2, // '3' key
};

struct FogSettings
{
	glm::vec3 color{ 1.0f, 1.0f, 1.0f };

	float start = 50.0f;
	float end = 200.0f;
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
	bool useFog = false;

	// fog controls
	FogSettings fogSettings;
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
private:
	int width_{};
	int height_{};

	RenderSettings renderSettings_;

	// passes
	GBufferPass gbuffer_;
	DebugPass debugPass_;
	SSAOPass ssaoPass_;
	FXAAPass fxaaPass_;
	FogPass fogPass_;
	PresentPass presentPass_;
	WaterPass waterPass_;

	uint32_t forwardFBO_{};
	uint32_t forwardColorTex_{};
	uint32_t forwardDepthTex_{};
};

#endif
