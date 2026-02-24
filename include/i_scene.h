#ifndef I_SCENE_H
#define I_SCENE_H

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

class IRenderer;
class RenderInputs;

class Camera;
class CubeMap;
class ChunkManager;
class ILight;

class IScene
{
public:
	virtual ~IScene() = default;

	 virtual void init() = 0;

	// render scene
	virtual void render(IRenderer& renderer, RenderInputs& in) = 0;

	// handle user inputs
	virtual void update(float dt, const InputState& in) = 0;

	// window events
	virtual void onResize(int w, int h) = 0;
	virtual void onMouseMove(float x, float y) = 0;
	virtual void onScroll(float yoffset) = 0;

	// saving feature
	virtual void requestSave() = 0;

	// getters
	virtual Camera& getCamera() = 0;
	virtual CubeMap& getSkybox() = 0;
	virtual ChunkManager& getWorld() = 0;
	virtual ILight& getLight() = 0;
};

#endif
