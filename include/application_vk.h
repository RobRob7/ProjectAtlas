#ifndef APPLICATION_VK_H
#define APPLICATION_VK_H

#include "i_renderer.h"

#include <memory>

struct GLFWwindow;

class Scene;
struct InputState;

class VulkanMain;

class ApplicationVk
{
public:
	ApplicationVk(int width, int height);
	~ApplicationVk();

	void run();

private:
	InputState buildInputState();
private:
	RenderInputs in_;

	// place/delete block limit
	bool leftMouseDown_ = false;
	bool rightMouseDown_ = false;

	int width_;
	int height_;

	GLFWwindow* window_ = nullptr;

	// time between current frame and last frame
	float deltaTime_ = 0.0f;
	float lastFrame_ = 0.0f;

	// save timer
	float saveTimer_ = 0.0f;
	// auto save time threshold (in min)
	float autoSaveTime_ = 5;

	bool framebufferResized_ = false;

	std::unique_ptr<VulkanMain> vk_;

	std::unique_ptr<Scene> scene_;
	std::unique_ptr<IRenderer> renderer_;
};
#endif