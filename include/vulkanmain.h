#ifndef VULKANMAIN_H
#define VULKANMAIN_H

#include <vulkan/vulkan.h>

#include <vulkan/vulkan_core.h>
#include <vulkan/vk_platform.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include <optional>
#include <set>
#include<string>
#include <algorithm>
#include <limits>

struct QueueFamilyIndices 
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails 
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


class VulkanMain
{
public:
	VulkanMain(GLFWwindow* window);
	~VulkanMain();

	void init();

private:
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createCommandPool();
	void createColorResources();
	void createDepthResources();

	void cleanup();
	void cleanupSwapChain();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger);
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	VkSampleCountFlagBits getMaxUsableSampleCount();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkImageView createImageView(VkImage image, VkFormat format,
		VkImageAspectFlags aspectFlags, uint32_t mipLevels);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilites);


private:
	const std::vector<const char*> validationLayers_ = {
		"VK_LAYER_KHRONOS_validation"
	};
	const std::vector<const char*> deviceExtensions_ = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	bool enableValidationLayers_{ false };

	VkSampleCountFlagBits msaaSamples_ = VK_SAMPLE_COUNT_1_BIT;

	GLFWwindow* window_;

	VkInstance instance_;
	VkDebugUtilsMessengerEXT debugMessenger_;
	VkSurfaceKHR surface_;

	VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
	VkDevice device_;

	VkQueue graphicsQueue_;
	VkQueue presentQueue_;

	VkSwapchainKHR swapChain_;
	std::vector<VkImage> swapChainImages_;
	VkFormat swapChainImageFormat_;
	VkExtent2D swapChainExtent_;
	std::vector<VkImageView> swapChainImageViews_;

	VkCommandPool commandPool_;
};

#endif
