#ifndef RENDER_SETTINGS_H
#define RENDER_SETTINGS_H

#include <glm/glm.hpp>

enum class DebugMode : int
{
	None = 0, // '1' key
	Normals = 1, // '2' key
	Depth = 2, // '3' key
};

struct FogSettings
{
	glm::vec3 color{ 1.0f, 1.0f, 1.0f };

	float start = 50.0f;
	float end = 500.0f;
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

#endif
