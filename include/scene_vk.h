#ifndef SCENE_VK_H
#define SCENE_VK_H

#include "i_scene.h"

#include <memory>

class VulkanMain;
class Camera;
class ICubemap;
class Crosshair;
class ChunkManager;
class ILight;

class IRenderer;
struct RenderInputs;

class SceneVk final : public IScene
{
public:
	SceneVk(VulkanMain& vk, int w, int h);
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

	// getters
	Camera& getCamera() override;
	ICubemap& getSkybox() override;
	ChunkManager& getWorld() override;
	ILight& getLight() override;

private:
	VulkanMain& vk_;
	// width of window
	int width_{};
	// height of window
	int height_{};

	// save timer
	float saveTimer_{ 0.0f };
	// auto save time threshold (in min)
	const float autoSaveTime_{ 5 };

	// objects
	std::unique_ptr<Camera> camera_;
	std::unique_ptr<ICubemap> skybox_;
	std::unique_ptr<Crosshair> crosshair_;
	std::unique_ptr<ChunkManager> world_;
	std::unique_ptr<ILight> light_;
};

#endif
