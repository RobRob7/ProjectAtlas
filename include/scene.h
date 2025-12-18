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

	// debug
	bool debugOffPressed = false;		// '1' key
	bool debugNormalPressed = false;	// '2' key
	bool debugDepthPressed = false;		// '3' key
};

class Scene
{
public:
	Scene(float w, float h);

	// setup scene entities
	void init();

	// render scene entities
	void render(float glfwTime);

	// handle user inputs
	void update(float dt, const InputState& in);

	// window events
	void onResize(float w, float h);
	void onMouseMove(float x, float y);
	void onScroll(float yoffset);

	// saving feature
	void requestSave();

private:
	// width of window
	float width_;
	// height of window
	float height_;

	// render pipeline
	Renderer renderer_;
	DebugMode debugMode_ = DebugMode::None;

	// objects
	std::optional<Camera> camera_;
	std::optional<CubeMap> skybox_;
	std::optional<Crosshair> crosshair_;
	std::optional<ChunkManager> world_;
	std::optional<Light> light_;
};

#endif
