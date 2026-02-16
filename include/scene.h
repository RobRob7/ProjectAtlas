#ifndef SCENE_H
#define SCENE_H

class Camera;
class CubeMap;
class Crosshair;
class ChunkManager;
class Light;

class Renderer;
struct RenderInputs;

#include <memory>

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
	bool enableImguiPressed = false;
	bool disableImguiPressed = false;
};

class Scene
{
public:
	Scene(int w, int h);
	~Scene();

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
	std::unique_ptr<Camera> camera_;
	std::unique_ptr<CubeMap> skybox_;
	std::unique_ptr<Crosshair> crosshair_;
	std::unique_ptr<ChunkManager> world_;
	std::unique_ptr<Light> light_;
};

#endif
