#ifndef APPLICATION_H
#define APPLICATION_H

#include "scene.h"
#include "renderer.h"
#include "ui.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <memory>
#include <algorithm>

class Application
{
public:
	Application(int width, int height, const char* windowTitle);
	~Application();

	void run();

private:
	InputState buildInputState();
private:
	RenderInputs in_;

	// place/delete block limit
	bool leftMouseDown_  = false;
	bool rightMouseDown_ = false;

	int width_;
	int height_;
	const char* windowTitle_;

	GLFWwindow* window_ = nullptr;

	// time between current frame and last frame
	float deltaTime_ = 0.0f;
	float lastFrame_ = 0.0f;

	// save timer
	float saveTimer_ = 0.0f;
	// auto save time threshold (in min)
	float autoSaveTime_ = 5;

	std::unique_ptr<Scene> scene_;
	std::unique_ptr<Renderer> renderer_;
	std::unique_ptr<UI> ui_;
};
#endif
