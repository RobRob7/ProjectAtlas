#ifndef APPLICATION_H
#define APPLICATION_H

#include "camera.h"
#include "shader.h"

#include "chunkmanager.h"
#include "cubemap.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <stdexcept>
#include <array>

class Application
{
public:
	Application(int width, int height, const char* windowTitle);
	~Application();

	void run();

private:
	void processInput();

private:
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

	// chunk manager
	ChunkManager world_{2};
};

#endif
