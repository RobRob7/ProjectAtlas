#ifndef RENDER_SETTINGS_H
#define RENDER_SETTINGS_H

#include <glm/glm.hpp>

enum class DebugMode : int
{
	None = 0,		// '1' key
	Normals = 1,	// '2' key
	Depth = 2,		// '3' key
	ShadowMap = 3,	// '4' key
	rtDepth = 4,	// '5' key
};

struct FogSettings
{
	bool volumetricFog = false;
	glm::vec3 color{ 1.0f, 1.0f, 1.0f };

	float start{ 50.0f };
	float end{ 175.0f };
};

struct RenderSettings
{
	// debug view mode
	DebugMode debugMode = DebugMode::None;

	// display options
	bool enableVsync = true;

	// graphics options
	bool useRT = false;

	bool useShadowMap = true;
	bool useSSAO = true;
	bool useFXAA = false;
	bool useFog = false;

	// fog controls
	FogSettings fogSettings;
};

#endif
