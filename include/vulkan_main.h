#ifndef VULKAN_MAIN_H
#define VULKAN_MAIN_H

#include <vulkan/vulkan.h>

#include <vector>
#include <cstdint>
#include <optional>

struct GLFWwindow;

struct VkFrameContext
{
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	uint32_t imageIndex = 0;

	VkImage swapchainImage = VK_NULL_HANDLE;
	VkImageView swapchainImageView = VK_NULL_HANDLE;
	VkExtent2D extent{};
	VkFormat format{};
};

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
	void waitIdle() const;

	bool beginFrame(VkFrameContext& out);
	bool endFrame(const VkFrameContext& frame);

	void notifyFramebufferResized() { framebufferResized_ = true; }

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
	void createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory) const;

	VkCommandBuffer beginSingleTimeCommands() const;
	void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;



	VkDevice device() const { return device_; }
	VkPhysicalDevice physicalDevice() const { return physicalDevice_; }
	VkQueue graphicsQueue() const { return graphicsQueue_; }
	VkQueue presentQueue() const { return presentQueue_; }
	VkCommandPool commandPool() const { return commandPool_; }

	VkFormat swapChainImageFormat() const { return swapChainImageFormat_; }
	VkExtent2D swapChainExtent() const { return swapChainExtent_; }
	const std::vector<VkImageView>& swapChainImageViews() const { return swapChainImageViews_; }
	const std::vector<VkImage>& swapChainImages() const { return swapChainImages_; }

	VkSwapchainKHR swapChain() const { return swapChain_; }

	VkSurfaceKHR surface() const { return surface_; }

	VkImageLayout swapchainLayout(uint32_t imageIndex) const { return swapchainLayouts_[imageIndex]; }
	void setSwapchainLayout(uint32_t imageIndex, VkImageLayout layout) { swapchainLayouts_[imageIndex] = layout; }

private:
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createCommandPool();

	void createCommandBuffers();
	void createSyncObjects();

	void cleanup();
	void cleanupSwapChain();
	void recreateSwapChain();

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

	void createPerImageSync();


private:
	const std::vector<const char*> validationLayers_ = {
		"VK_LAYER_KHRONOS_validation"
	};
	const std::vector<const char*> deviceExtensions_ = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	bool enableValidationLayers_{ false };

	bool initialized_{ false };

	static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	bool framebufferResized_{ false };

	uint32_t currentFrame_ = 0;

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
	std::vector<VkImageLayout> swapchainLayouts_;

	VkCommandPool commandPool_;

	std::vector<VkCommandBuffer> commandBuffers_;
	std::vector<VkSemaphore> imageAvailableSemaphores_;
	std::vector<VkFence> inFlightFences_;

	std::vector<VkSemaphore> renderFinishedPerImage_;
	std::vector<VkFence> imagesInFlight_;
};

#endif
