#include "scene.h"

//--- PUBLIC ---//
Scene::Scene(float w, float h)
	: width_(w), height_(h)
{
} // end of constuctor

void Scene::init()
{
	// init renderer
	renderer_.init();
	renderer_.resize(width_, height_);

	camera_.emplace(width_, height_, glm::vec3(0.0f, 100.0f, 3.0f));

	skybox_.emplace();
	skybox_->init();

	world_.emplace(12);
	world_->init();

	crosshair_.emplace();
	crosshair_->init();

	light_.emplace(camera_->getCameraPosition());
	light_->init();
} // end of init

void Scene::render(RenderInputs& in)
{
	if (!camera_ || !world_ || !light_ || !skybox_ || !crosshair_) return;

	renderer_.resize(width_, height_);

	in.world = &*world_;
	in.camera = &*camera_;
	in.light = &*light_;
	in.skybox = &*skybox_;
	in.crosshair = &*crosshair_;

	renderer_.renderFrame(in);
} // end of render()

void Scene::update(float dt, const InputState& in)
{
	if (!camera_ || !world_) return;

	if (in.quitRequested)
	{
		requestSave();
		return;
	}

	if (in.disableCameraPressed)
	{
		camera_->setEnabled(false);
	}

	if (in.enableCameraPressed)
	{
		camera_->setEnabled(true);
	}

	// functionality ONLY when camera active
	if (camera_->isEnabled())
	{
		camera_->setAccelerationMultiplier(in.sprint ? 15.0f : 1.0f);

		if (in.w) camera_->processKeyboard(FORWARD, dt);
		if (in.a) camera_->processKeyboard(LEFT, dt);
		if (in.s) camera_->processKeyboard(BACKWARD, dt);
		if (in.d) camera_->processKeyboard(RIGHT, dt);

		if (in.removeBlockPressed)
		{
			world_->placeOrRemoveBlock(false,
				camera_->getCameraPosition(),
				camera_->getCameraDirection());
		}

		if (in.placeBlockPressed)
		{
			world_->placeOrRemoveBlock(true,
				camera_->getCameraPosition(),
				camera_->getCameraDirection());
		}
	}
} // end of update()

void Scene::onResize(float w, float h)
{
	width_ = w;
	height_ = h;
} // end of onResize()

void Scene::onMouseMove(float x, float y)
{
	if (camera_ && camera_->isEnabled())
	{
		camera_->handleMousePosition(x, y);
	}
} // end of onMouseMove()

void Scene::onScroll(float yoffset)
{
	if (camera_ && camera_->isEnabled())
	{
		camera_->handleMouseScroll(yoffset);
	}
} // end of onScroll()

void Scene::requestSave()
{
	if (world_)
	{
		world_->saveWorld();
	}
} // end of requestSave()

Camera& Scene::getCamera()
{
	return *camera_;
} // end of getCamera()

CubeMap& Scene::getSkybox()
{
	return *skybox_;
} // end of getSkybox()

ChunkManager& Scene::getWorld()
{
	return *world_;
} // end of getWorld()

Light& Scene::getLight()
{
	return *light_;
} // end of getLight()

Renderer& Scene::getRenderer()
{
	return renderer_;
} // end of getRenderer()
