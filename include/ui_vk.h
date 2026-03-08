#ifndef UI_VK_H
#define UI_VK_H

#include "vulkan/vulkan.hpp"

#include <memory>

class IScene;
struct GLFWwindow;
struct RenderSettings;
class VulkanMain;

constexpr float TOP_BAR_HEIGHT1 = 36.0f;
constexpr float INSPECTOR_WIDTH1 = 400.0f;

class UIVk
{
public:
	UIVk(VulkanMain& vk, GLFWwindow* window, RenderSettings& rs);
	~UIVk();

	void beginFrame();
	void buildUI(float dt, IScene& scene);
	void render(vk::CommandBuffer cmd);
	void setUIInputEnabled(bool enabled);
	void setUIDisplayEnabled(bool enabled);
	void setCameraModeUIEnabled(bool enabled);

private:
	void drawTopBar();
	void drawStatsFPS(float dt);
	void drawInspector(IScene& scene);
private:
	VulkanMain& vk_;

	GLFWwindow* window_;
	RenderSettings& renderSettings_;

	bool enabled_;
	bool cameraModeOn_;
};

#endif