#ifndef APPLICATION_H
#define APPLICATION_H

#include "camera.h"
#include "shader.h"

#include "chunkmanager.h"
#include "cubemap.h"
#include "crosshair.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <stdexcept>
#include <array>
#include <cmath>

class Application
{
public:
	Application(int width, int height, const char* windowTitle);
	~Application();

	void run();

private:
	void processInput();

private:
	// place/delete block limit
	bool leftMouseDown_  = false;
	bool rightMouseDown_ = false;
	// width of window
	float width_;
	// height of window
	float height_;
	// window title name
	const char* windowTitle_;
	// GLFWwindow instance
	GLFWwindow* window_ = nullptr;

	// time between current frame and last frame
	float deltaTime_ = 0.0f;
	// time of last frame
	float lastFrame_ = 0.0f;

	// camera system
	std::unique_ptr<Camera> camera_;

	// skybox
	std::unique_ptr<CubeMap> skybox_;

	// crosshair
	std::unique_ptr<Crosshair> crosshair_;

	// chunk manager
	ChunkManager world_{12};

	// save timer
	float saveTimer_ = 0.0f;
	// auto save time threshold (in min)
	float autoSaveTime_ = 5;

	// light position + color
	glm::vec3 lightPos_{0.0f, 15.0f, 3.0f};
	glm::vec3 lightColor_{ 1.0f, 1.0f, 1.0f };
};

#endif
