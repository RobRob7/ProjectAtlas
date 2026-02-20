#ifndef RENDERINPUTS_H
#define RENDERINPUTS_H

class ChunkManager;
class Camera;
class Light;
class CubeMap;
class Crosshair;

struct RenderInputs
{
	ChunkManager* world = nullptr;
	Camera* camera = nullptr;
	Light* light = nullptr;
	CubeMap* skybox = nullptr;
	Crosshair* crosshair = nullptr;

	float time = 0.0f;
};

#endif
