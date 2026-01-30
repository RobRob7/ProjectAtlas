#ifndef APPLICATION_H
#define APPLICATION_H

#include "scene.h"
#include "renderer.h"
#include "texture.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <stdexcept>
#include <algorithm>
#define NOMINMAX
#include <windows.h>
#include <psapi.h>

inline constexpr float TOP_BAR_HEIGHT = 36.0f;
inline constexpr float INSPECTOR_WIDTH = 400.0f;

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
};

size_t GetProcessMemoryMB();
#endif
