#ifndef UI_H
#define UI_H

#include <vulkan/vulkan.h>

#include <cstdint>




#include <memory>

class Texture;
class Scene;
struct GLFWwindow;
struct RenderSettings;

constexpr float TOP_BAR_HEIGHT = 36.0f;
constexpr float INSPECTOR_WIDTH = 400.0f;

class UI
{
public:
	UI(GLFWwindow* window);
	~UI();

	void initVulkan(
		VkInstance instance,
		VkPhysicalDevice physDev,
		VkDevice device,
		uint32_t graphicsQueueFamily,
		VkQueue graphicsQueue,
		VkRenderPass renderPass,
		uint32_t swapchainImageCount,
		VkCommandPool commandPool,
		VkDescriptorPool imguiPool
		);

	void init();
	void drawFullUI(float dt, Scene& scene, RenderSettings& rs);
	void setUIInputEnabled(bool enabled);
	void setUIDisplayEnabled(bool enabled);
	void setCameraModeUIEnabled(bool enabled);

	void drawTopBar();
private:
	//void drawTopBar();
	void drawStatsFPS(float dt);
	void drawInspector(Scene& scene, RenderSettings& rs);
private:
	GLFWwindow* window_;

	VkDevice device_{ VK_NULL_HANDLE };
	VkDescriptorPool imguiPool_{ VK_NULL_HANDLE };

	// window top bar logo
	std::unique_ptr<Texture> logoTex_;

	bool enabled_{ true };
	bool cameraModeOn_{ true };
};

#endif
