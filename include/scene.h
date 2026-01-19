#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "cubemap.h"
#include "crosshair.h"
#include "chunkmanager.h"
#include "light.h"

#include "renderer.h"

#include <optional>

struct InputState
{
	// keys
	bool w = false;
	bool a = false;
	bool s = false;
	bool d = false;
	bool sprint = false;

	// actions
	bool enableCameraPressed = false;
	bool disableCameraPressed = false;
	bool placeBlockPressed = false;
	bool removeBlockPressed = false;
	bool quitRequested = false;
};

class Scene
{
public:
	Scene(float w, float h);

	// setup scene with renderer
	void init(Renderer& renderer);

	// render scene
	void render(Renderer& renderer, RenderInputs& in);

	// handle user inputs
	void update(float dt, const InputState& in);

	// window events
	void onResize(int w, int h);
	void onMouseMove(float x, float y);
	void onScroll(float yoffset);

	// saving feature
	void requestSave();

	// getters
	Camera& getCamera();
	CubeMap& getSkybox();
	ChunkManager& getWorld();
	Light& getLight();

private:
	// width of window
	int width_;
	// height of window
	int height_;

	// objects
	std::optional<Camera> camera_;
	std::optional<CubeMap> skybox_;
	std::optional<Crosshair> crosshair_;
	std::optional<ChunkManager> world_;
	std::optional<Light> light_;
};

#endif
