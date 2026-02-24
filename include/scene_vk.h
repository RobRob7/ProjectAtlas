#ifndef SCENE_VK_H
#define SCENE_VK_H

#include "i_scene.h"

#include <memory>

class Camera;
class CubeMap;
class Crosshair;
class ChunkManager;
class ILight;

class IRenderer;
struct RenderInputs;

class SceneVk final : public IScene
{
public:
	SceneVk(int w, int h);
	~SceneVk() override;

	void init() override;

	// render scene
	void render(IRenderer& renderer, RenderInputs& in) override;

	// handle user inputs
	void update(float dt, const InputState& in) override;

	// window events
	void onResize(int w, int h) override;
	void onMouseMove(float x, float y) override;
	void onScroll(float yoffset) override;

	// saving feature
	void requestSave() override;

	// getters
	Camera& getCamera() override;
	CubeMap& getSkybox() override;
	ChunkManager& getWorld() override;
	ILight& getLight() override;

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
	std::unique_ptr<ILight> light_;
};

#endif
