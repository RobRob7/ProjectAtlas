#ifndef SCENE_H
#define SCENE_H

#include "camera.h"
#include "cubemap.h"
#include "crosshair.h"
#include "chunkmanager.h"
#include "light.h"

#include <optional>

class Scene
{
public:
	Scene(float w, float h);

	void init();
	void render(float w, float h, float glfwTime);

	void onResize(float w, float h);
	void onMouseMove(float x, float y);
	void onScroll(float yoffset);

	void saveWorld();

	void setCameraEnabled(bool enabled);
	bool isCameraEnabled() const;

	void setSpeedMultiplier(float m);

	void placeOrRemoveBlock(bool place);
	void moveForward(float dt);
	void moveBackward(float dt);
	void moveLeft(float dt);
	void moveRight(float dt);

private:
	// width of window
	float width_;
	// height of window
	float height_;

	// camera system
	std::optional<Camera> camera_;

	// skybox
	std::optional<CubeMap> skybox_;

	// crosshair
	std::optional<Crosshair> crosshair_;

	// chunk manager
	std::optional<ChunkManager> world_;

	// light
	std::optional<Light> light_;
};

#endif
