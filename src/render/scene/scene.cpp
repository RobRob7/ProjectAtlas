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

void Scene::render(float w, float h, float glfwTime)
{
	// update width, height
	width_ = w;
	height_ = h;

	// update world
	world_->update(camera_->getCameraPosition());

	// set color to display after clear (state-setting function)
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	// clear the screen colors (state-using function)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//////////////////////////////
	// init transform matrices
	glm::mat4 view = camera_->getViewMatrix();
	glm::mat4 projection = camera_->getProjectionMatrix(width_ / height_);

	world_->getShader()->use();
	world_->getShader()->setVec3("u_viewPos", camera_->getCameraPosition());
	world_->getShader()->setVec3("u_lightPos", light_->getPosition());
	world_->getShader()->setVec3("u_lightColor", light_->getColor());

	// render world
	world_->render(view, projection);
	light_->render(view, projection);

	// render skybox
	skybox_->render(view, projection, glfwTime);

	// render crosshair
	crosshair_->render();
} // end of render()

void Scene::onResize(float w, float h)
{
	width_ = w;
	height_ = h;
} // end of onResize()

void Scene::onMouseMove(float x, float y)
{
	if (camera_)
	{
		camera_->handleMousePosition(x, y);
	}
} // end of onMouseMove()

void Scene::onScroll(float yoffset)
{
	if (camera_)
	{
		camera_->handleMouseScroll(yoffset);
	}
} // end of onScroll()

void Scene::saveWorld()
{
	if (world_)
	{
		world_->saveWorld();
	}
} // end of saveWorld()

void Scene::setCameraEnabled(bool enabled)
{
	if (camera_)
	{
		camera_->setEnabled(enabled);
	}
} // end of setCameraEnabled()

bool Scene::isCameraEnabled() const
{
	return camera_ && camera_->isEnabled();
} // end of isCameraEnabled()

void Scene::setSpeedMultiplier(float m)
{
	if (camera_)
	{
		camera_->setAccelerationMultiplier(m);
	}
} // end of setSpeedMultiplier()

void Scene::placeOrRemoveBlock(bool place)
{
	if (!camera_ || !world_) return;

	world_->placeOrRemoveBlock(place,
		camera_->getCameraPosition(),
		camera_->getCameraDirection());
} // end of placeOrRemoveBlock()

void Scene::moveForward(float dt)
{
	if (camera_)
	{
		camera_->processKeyboard(FORWARD, dt);
	}
} // end of moveForward()

void Scene::moveBackward(float dt)
{
	if (camera_)
	{
		camera_->processKeyboard(BACKWARD, dt);
	}
} // end of moveBackward()

void Scene::moveLeft(float dt)
{
	if (camera_)
	{
		camera_->processKeyboard(LEFT, dt);
	}
} // end of moveLeft()

void Scene::moveRight(float dt)
{
	if (camera_)
	{
		camera_->processKeyboard(RIGHT, dt);
	}
} // end of moveRight()