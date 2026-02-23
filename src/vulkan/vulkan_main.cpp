#include "vulkan_main.h"

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <iostream>
#include <set>
#include <string>
#include <algorithm>
#include <limits>

//--- HELPER ---//
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
} // end of debugCallback()

static void DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator) 
{

	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) 
	{
		func(instance, debugMessenger, pAllocator);
	}
} // end of DestroyDebugUtilsMessengerEXT()


//--- PUBLIC ---//
VulkanMain::VulkanMain(GLFWwindow* window)
	: window_(window)
{
#ifndef NDEBUG
enableValidationLayers_ = true;
#endif
} // end of constructor

VulkanMain::~VulkanMain()
{
	if (initialized_)
	{
		cleanup();
	}
} // end of destructor

void VulkanMain::init()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createCommandPool();
	createCommandBuffers();
	createSyncObjects();

	initialized_ = true;
} // end of init()

void VulkanMain::waitIdle() const
{
	if (device_)
	{
		vkDeviceWaitIdle(device_);
	}
} // end of waitIdle()

bool VulkanMain::beginFrame(VkFrameContext& out)
{
	vkWaitForFences(device_, 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex = 0;
	VkResult res = vkAcquireNextImageKHR(
		device_,
		swapChain_,
		UINT64_MAX,
		imageAvailableSemaphores_[currentFrame_],
		VK_NULL_HANDLE,
		&imageIndex
	);

	if (res == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain();
		return false;
	}
	if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swapchain image!");
	}

	if (!imagesInFlight_.empty() && imagesInFlight_[imageIndex] != VK_NULL_HANDLE)
	{
		vkWaitForFences(device_, 1, &imagesInFlight_[imageIndex], VK_TRUE, UINT64_MAX);
	}
	if (!imagesInFlight_.empty())
	{
		imagesInFlight_[imageIndex] = inFlightFences_[currentFrame_];
	}

	vkResetFences(device_, 1, &inFlightFences_[currentFrame_]);

	VkCommandBuffer cmd = commandBuffers_[currentFrame_];
	vkResetCommandBuffer(cmd, 0);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin command buffer!");
	}

	// fill out frame context
	out.cmd = cmd;
	out.imageIndex = imageIndex;
	out.swapchainImage = swapChainImages_[imageIndex];
	out.swapchainImageView = swapChainImageViews_[imageIndex];
	out.extent = swapChainExtent_;
	out.format = swapChainImageFormat_;

	return true;
} // end of beginFrame()

bool VulkanMain::endFrame(const VkFrameContext& frame)
{
	if (vkEndCommandBuffer(frame.cmd) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record command buffer!");
	}

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores_[currentFrame_] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	VkSemaphore signalSemaphores[] = { renderFinishedPerImage_[frame.imageIndex] };

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &frame.cmd;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, inFlightFences_[currentFrame_]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkSwapchainKHR swapChains[] = { swapChain_ };
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &frame.imageIndex;

	VkResult res = vkQueuePresentKHR(presentQueue_, &presentInfo);

	bool needRecreate =
		framebufferResized_ ||
		res == VK_ERROR_OUT_OF_DATE_KHR ||
		res == VK_SUBOPTIMAL_KHR;

	if (needRecreate)
	{
		framebufferResized_ = false;
		recreateSwapChain();
		currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
		return false;
	}

	if (res != VK_SUCCESS)
	{
		std::cerr << "vkQueuePresentKHR failed: " << res << "\n";
		throw std::runtime_error("failed to present swapchain image!");
	}

	currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
	return true;
} // end of endFrame()

uint32_t VulkanMain::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) 
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
		{
			return i;
		}
	} // end for

	throw std::runtime_error("failed to find suitable memory type!");
} // end of findMemoryType()

void VulkanMain::createBuffer(
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkBuffer& buffer,
	VkDeviceMemory& bufferMemory) const
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device_, buffer, bufferMemory, 0);
} // end of createBuffer()

VkCommandBuffer VulkanMain::beginSingleTimeCommands() const
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool_;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
} // end of beginSingleTimeCommands()

void VulkanMain::endSingleTimeCommands(VkCommandBuffer commandBuffer) const
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue_);

	vkFreeCommandBuffers(device_, commandPool_, 1, &commandBuffer);
} // end of endSingleTimeCommands()

void VulkanMain::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
} // end of copyBuffer()


//--- PRIVATE ---//
void VulkanMain::createInstance()
{
	// check for validation layers
	if (enableValidationLayers_ && !checkValidationLayerSupport()) 
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// fill in struct with application info
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_4;

	// fill in struct to tell Vulkan driver which global extensions
	// and validation layers we want to use
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers_) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers_.size());
		createInfo.ppEnabledLayerNames = validationLayers_.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = &debugCreateInfo;
	}
	else 
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// check if instance was created successfully
	if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create instance!");
	}
} // end of createInstance()

void VulkanMain::setupDebugMessenger()
{
	if (!enableValidationLayers_) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debugMessenger_) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to set up debug messenger");
	}
} // end of setupDebugMessenger()

void VulkanMain::createSurface()
{
	if (glfwCreateWindowSurface(instance_, window_, nullptr, &surface_) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create window surface!");
	}
} // end of createSurface()

void VulkanMain::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);

	if (deviceCount == 0) 
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
	for (const auto& device : devices) 
	{
		if (isDeviceSuitable(device)) 
		{
			physicalDevice_ = device;
			msaaSamples_ = getMaxUsableSampleCount();
			break;
		}
	} // end for

	if (physicalDevice_ == VK_NULL_HANDLE) 
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
} // end of pickPhysicalDevice()

void VulkanMain::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) 
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	} // end for

	VkPhysicalDeviceFeatures2 deviceFeatures2{};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.features.samplerAnisotropy = VK_TRUE;
	deviceFeatures2.features.sampleRateShading = VK_TRUE;
	VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering{};
	dynamicRendering.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
	dynamicRendering.dynamicRendering = VK_TRUE;
	deviceFeatures2.pNext = &dynamicRendering;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = nullptr;
	createInfo.pNext = &deviceFeatures2;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions_.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions_.data();

	if (enableValidationLayers_) 
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers_.size());
		createInfo.ppEnabledLayerNames = validationLayers_.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
	vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);
} // end of createLogicalDevice()

void VulkanMain::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice_);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 &&
		imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface_;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else 
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapChain_) != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, nullptr);
	swapChainImages_.resize(imageCount);
	swapchainLayouts_.assign(swapChainImages_.size(), VK_IMAGE_LAYOUT_UNDEFINED);
	vkGetSwapchainImagesKHR(device_, swapChain_, &imageCount, swapChainImages_.data());
	swapChainImageFormat_ = surfaceFormat.format;
	swapChainExtent_ = extent;
} // end of createSwapChain()

void VulkanMain::createImageViews()
{
	swapChainImageViews_.resize(swapChainImages_.size());

	for (uint32_t i = 0; i < swapChainImages_.size(); ++i) 
	{
		swapChainImageViews_[i] = createImageView(swapChainImages_[i], swapChainImageFormat_,
			VK_IMAGE_ASPECT_COLOR_BIT, 1);
	} // end for
} // end of createImageViews()

void VulkanMain::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice_);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
} // end of createCommandPool()

void VulkanMain::createCommandBuffers()
{
	commandBuffers_.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool_;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());

	if (vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers_.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}
} // end of createCommandBuffers()

void VulkanMain::createSyncObjects()
{
	// per-frame sync
	imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semInfo{};
	semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (vkCreateSemaphore(device_, &semInfo, nullptr, &imageAvailableSemaphores_[i]) != VK_SUCCESS ||
			vkCreateFence(device_, &fenceInfo, nullptr, &inFlightFences_[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create per-frame sync objects!");
		}
	} // end for

	createPerImageSync();
} // end of createSyncObjects()

void VulkanMain::cleanup()
{
	if (!initialized_) return;

	cleanupSwapChain();

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (imageAvailableSemaphores_[i]) vkDestroySemaphore(device_, imageAvailableSemaphores_[i], nullptr);
		if (inFlightFences_[i]) vkDestroyFence(device_, inFlightFences_[i], nullptr);
	} // end for
	imageAvailableSemaphores_.clear();
	inFlightFences_.clear();

	// destroy command pool
	if (commandPool_)
	{
		vkDestroyCommandPool(device_, commandPool_, nullptr);
		commandPool_ = VK_NULL_HANDLE;
	}

	// destroy device
	if (device_)
	{
		vkDestroyDevice(device_, nullptr);
		device_ = VK_NULL_HANDLE;
	}

	// destroy debug messenger
	if (enableValidationLayers_ && debugMessenger_) 
	{
		DestroyDebugUtilsMessengerEXT(instance_, debugMessenger_, nullptr);
		debugMessenger_ = VK_NULL_HANDLE;
	}

	// destroy surface
	if (surface_)
	{
		vkDestroySurfaceKHR(instance_, surface_, nullptr);
		surface_ = VK_NULL_HANDLE;
	}

	// destroy vulkan instance
	if (instance_)
	{
		vkDestroyInstance(instance_, nullptr);
		instance_ = VK_NULL_HANDLE;
	}

	initialized_ = false;
} // end of cleanup()

void VulkanMain::cleanupSwapChain()
{
	if (!device_) return;

	for (VkSemaphore s : renderFinishedPerImage_)
	{
		if (s)
		{
			vkDestroySemaphore(device_, s, nullptr);
		}
	} // end for
	renderFinishedPerImage_.clear();
	imagesInFlight_.clear();

	for (size_t i = 0; i < swapChainImageViews_.size(); ++i) 
	{
		vkDestroyImageView(device_, swapChainImageViews_[i], nullptr);
	} // end for
	swapChainImageViews_.clear();

	if (swapChain_)
	{
		vkDestroySwapchainKHR(device_, swapChain_, nullptr);
		swapChain_ = VK_NULL_HANDLE;
	}

	swapChainImages_.clear();
	swapchainLayouts_.clear();
	swapChainImageFormat_ = VK_FORMAT_UNDEFINED;
	swapChainExtent_ = {};
} // end of cleanupSwapChain()

void VulkanMain::recreateSwapChain()
{
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window_, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window_, &width, &height);
		glfwWaitEvents();
	} // end while

	vkDeviceWaitIdle(device_);

	cleanupSwapChain();
	createSwapChain();
	createImageViews();

	createPerImageSync();

	framebufferResized_ = false;
} // end of recreateSwapChain()


void VulkanMain::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
} // end of populateDebugMessengerCreateInfo()

VkResult VulkanMain::CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger) {

	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
} // end of CreateDebugUtilsMessengerEXT()

bool VulkanMain::checkValidationLayerSupport()
{
	// list all available layers
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers_)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		} // end for

		if (!layerFound)
		{
			return false;
		}
	} // end for

	return true;
} // end of checkValidationLayerSupport()

std::vector<const char*> VulkanMain::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers_)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
} // end of getRequiredExtensions()

bool VulkanMain::isDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceDynamicRenderingFeatures dyn{};
	dyn.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;

	VkPhysicalDeviceFeatures2 feats2{};
	feats2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	feats2.pNext = &dyn;

	vkGetPhysicalDeviceFeatures2(device, &feats2);

	return indices.isComplete() 
		&& extensionsSupported
		&& swapChainAdequate 
		&& feats2.features.samplerAnisotropy 
		&& feats2.features.sampleRateShading 
		&& dyn.dynamicRendering;
} // end of isDeviceSuitable()

QueueFamilyIndices VulkanMain::findQueueFamilies(VkPhysicalDevice device) 
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) 
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
		if (presentSupport) 
		{
			indices.presentFamily = i;
		}
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
		{
			indices.graphicsFamily = i;
		}
		if (indices.isComplete()) 
		{
			break;
		}
		i++;
	} // end for
	return indices;
}

VkSampleCountFlagBits VulkanMain::getMaxUsableSampleCount() 
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice_, &physicalDeviceProperties);

	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
		physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & VK_SAMPLE_COUNT_64_BIT) 
	{
		return VK_SAMPLE_COUNT_64_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_32_BIT) 
	{
		return VK_SAMPLE_COUNT_32_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_16_BIT) 
	{
		return VK_SAMPLE_COUNT_16_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_8_BIT) 
	{
		return VK_SAMPLE_COUNT_8_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_4_BIT) 
	{
		return VK_SAMPLE_COUNT_4_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_2_BIT) 
	{
		return VK_SAMPLE_COUNT_2_BIT;
	}

	return VK_SAMPLE_COUNT_1_BIT;
} // end of getMaxUsableSampleCount()

bool VulkanMain::checkDeviceExtensionSupport(VkPhysicalDevice device) 
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions_.begin(), deviceExtensions_.end());

	for (const auto& extension : availableExtensions) 
	{
		requiredExtensions.erase(extension.extensionName);
	} // end for

	return requiredExtensions.empty();
} // end of checkDeviceExtensionSupport()

SwapChainSupportDetails VulkanMain::querySwapChainSupport(VkPhysicalDevice device) 
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

	if (formatCount != 0) 
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

	if (presentModeCount != 0) 
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
	}

	return details;
} // end of querySwapChainSupport()

VkImageView VulkanMain::createImageView(VkImage image, VkFormat format,
	VkImageAspectFlags aspectFlags, uint32_t mipLevels) 
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device_, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}

	return imageView;
} // end of createImageView()

VkSurfaceFormatKHR VulkanMain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) 
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
		{
			return availableFormat;
		}
	} // end for

	return availableFormats[0];
} // end of chooseSwapSurfaceFormat()

VkPresentModeKHR VulkanMain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) 
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) 
		{
			return availablePresentMode;
		}
	} // end for
	return VK_PRESENT_MODE_FIFO_KHR;
} // end of chooseSwapPresentMode()

VkExtent2D VulkanMain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilites)
{
	if (capabilites.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
	{
		return capabilites.currentExtent;
	}
	else 
	{
		int width, height;
		glfwGetFramebufferSize(window_, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width,
			capabilites.minImageExtent.width,
			capabilites.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height,
			capabilites.minImageExtent.height,
			capabilites.maxImageExtent.height);

		return actualExtent;
	}
} // end of chooseSwapExtent()

void VulkanMain::createPerImageSync()
{
	// per-swapchain image sync
	renderFinishedPerImage_.resize(swapChainImages_.size());
	imagesInFlight_.assign(swapChainImages_.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semInfo{};
	semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (uint32_t i = 0; i < swapChainImages_.size(); ++i)
	{
		if (vkCreateSemaphore(device_, &semInfo, nullptr, &renderFinishedPerImage_[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create per-image render finished semaphores!");
		}
	} // end for
} // end of createPerImageSync()