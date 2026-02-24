#ifndef RENDER_INPUTS_H
#define RENDER_INPUTS_H

class Camera;
class ChunkManager;
class ILight;
class CubeMap;
class Crosshair;

struct RenderInputs
{
	Camera* camera = nullptr;
	ChunkManager* world = nullptr;
	ILight* light = nullptr;
	CubeMap* skybox = nullptr;
	Crosshair* crosshair = nullptr;

	float time = 0.0f;
};

#endif
