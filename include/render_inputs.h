#ifndef RENDER_INPUTS_H
#define RENDER_INPUTS_H

class Camera;
class ChunkManager;
class ILight;
class ICubemap;
class ICrosshair;

struct RenderInputs
{
	Camera* camera = nullptr;
	ChunkManager* world = nullptr;
	ILight* light = nullptr;
	ICubemap* skybox = nullptr;
	ICrosshair* crosshair = nullptr;

	float time = 0.0f;
};

#endif
