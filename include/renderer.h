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

#include <glm/glm.hpp>
#include <glad/glad.h>

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
	void resize(float w, float h);
	void renderFrame(const RenderInputs& in);

	RenderSettings& settings();

private:
	float width_{};
	float height_{};

	// passes
	GBufferPass gbuffer_;
	DebugPass debugPass_;
	SSAOPass ssaoPass_;

	RenderSettings renderSettings_;
};

#endif
