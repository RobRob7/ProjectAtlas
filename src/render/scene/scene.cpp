#include "scene.h"

//--- PUBLIC ---//
Scene::Scene(float w, float h)
	: width_(w), height_(h)
{
} // end of constuctor

void Scene::init()
{
	// camera controller
	camera_.emplace(width_, height_, glm::vec3(0.0f, 100.0f, 3.0f));

	// cubemap
	skybox_.emplace();
	skybox_->init();

	// setup shader + texture
	world_.emplace(12);
	world_->init();

	// crosshair
	crosshair_.emplace();
	crosshair_->init();

	// light
	light_.emplace(camera_->getCameraPosition());
	light_->init();
} // end of init

void Scene::render(float glfwTime)
{
	if (!camera_ || !world_ || !light_ || !skybox_ || !crosshair_) return;

	// update world
	world_->update(camera_->getCameraPosition());

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view = camera_->getViewMatrix();
	glm::mat4 projection = camera_->getProjectionMatrix(width_ / height_);

	// update uniforms of world shader
	auto& worldShader = world_->getShader();
	worldShader->use();
	worldShader->setVec3("u_viewPos", camera_->getCameraPosition());
	worldShader->setVec3("u_lightPos", light_->getPosition());
	worldShader->setVec3("u_lightColor", light_->getColor());

	world_->render(view, projection);
	light_->render(view, projection);

	skybox_->render(view, projection, glfwTime);
	crosshair_->render();
} // end of render()

void Scene::update(float dt, const InputState& in)
{
	if (!camera_ || !world_) return;

	if (in.quitRequested)
	{
		world_->saveWorld();
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