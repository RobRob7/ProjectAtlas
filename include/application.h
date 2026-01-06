#ifndef APPLICATION_H
#define APPLICATION_H

#include "scene.h"
#include "texture.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>

struct RenderSettings
{
	bool useSSAO = false;
};

class Application
{
public:
	Application(int width, int height, const char* windowTitle);
	~Application();

	void run();

private:
	InputState buildInputState();

	void drawFullUI();

	void drawTopBar(GLFWwindow* window, ImTextureID logoTex);
	void drawStatsFPS();
	void drawInspector();
private:
	// window top bar logo
	uint32_t logoTex_;

	RenderInputs in_;

	// graphics options
	RenderSettings renderSettings_;

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

	// save timer
	float saveTimer_ = 0.0f;
	// auto save time threshold (in min)
	float autoSaveTime_ = 5;

	std::unique_ptr<Scene> scene_;
};
#endif
