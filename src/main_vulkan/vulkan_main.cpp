#include "vulkan_main.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <iostream>
#include <set>
#include <string>
#include <algorithm>
#include <limits>
#include <utility>
#include <cstring>

//--- HELPER ---//
static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	vk::DebugUtilsMessageTypeFlagsEXT messageType,
	const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
} // end of debugCallback()


//--- PUBLIC ---//
VulkanMain::VulkanMain(GLFWwindow* window)
	: window_(window)
{
#ifdef _DEBUG
	enableValidationLayers_ = true;
#endif
} // end of constructor

VulkanMain::~VulkanMain() = default;

void VulkanMain::init()
{
	if (initialized_) return;

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
		device_->waitIdle();
	}
} // end of waitIdle()

bool VulkanMain::beginFrame(FrameContext& out)
{
	// wait for this frame fence
	{
		vk::Fence f = inFlightFences_[currentFrame_].get();
		vk::Result res = device_->waitForFences(1, &f, VK_TRUE, UINT64_MAX);
		if (res != vk::Result::eSuccess)
		{
			throw std::runtime_error("waitForFences failed!");
		}
	}

	// acquire next image
	uint32_t imageIndex = 0;
	{
		auto rv = device_->acquireNextImageKHR(
			swapChain_.get(),
			UINT64_MAX,
			imageAvailableSemaphores_[currentFrame_].get(),
			nullptr
		);

		if (rv.result == vk::Result::eErrorOutOfDateKHR)
		{
			recreateSwapChain();
			return false;
		}
		if (rv.result != vk::Result::eSuccess && rv.result != vk::Result::eSuboptimalKHR)
		{
			throw std::runtime_error("failed to acquire swapchain image!");
		}

		imageIndex = rv.value;
	}

	// if a prev frame is using this image, wait on that fence
	if (!imagesInFlight_.empty() && imagesInFlight_[imageIndex])
	{
		vk::Fence imgFence = imagesInFlight_[imageIndex];
		vk::Result res = device_->waitForFences(1, &imgFence, VK_TRUE, UINT64_MAX);

		if (res != vk::Result::eSuccess)
		{
			throw std::runtime_error("waitForFences(imagesInFlight) failed!");
		}
	}

	// mark the image as now being in flight with this frame fence
	if (!imagesInFlight_.empty())
	{
		imagesInFlight_[imageIndex] = inFlightFences_[currentFrame_].get();
	}

	// reset this frame fence for upcoming submit
	{
		vk::Fence f = inFlightFences_[currentFrame_].get();
		vk::Result res = device_->resetFences(1, &f);

		if (res != vk::Result::eSuccess)
		{
			throw std::runtime_error("resetFences failed!");
		}
	}

	// reset + begin command buffer
	vk::CommandBuffer cmd = commandBuffers_[currentFrame_];
	cmd.reset(vk::CommandBufferResetFlags{});

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	if (cmd.begin(beginInfo) != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to begin command buffer!");
	}

	// fill out frame context
	out.cmd = cmd;
	out.frameIndex = currentFrame_;
	out.imageIndex = imageIndex;
	out.swapchainImage = swapChainImages_[imageIndex];
	out.swapchainImageView = swapChainImageViews_[imageIndex].get();
	out.extent = swapChainExtent_;
	out.format = swapChainImageFormat_;

	return true;
} // end of beginFrame()

bool VulkanMain::endFrame(const FrameContext& frame)
{
	if (frame.cmd.end() != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to record command buffer!");
	}

	vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores_[currentFrame_].get()};
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	vk::Semaphore signalSemaphores[] = { renderFinishedPerImage_[frame.imageIndex].get()};

	vk::SubmitInfo submitInfo{};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &frame.cmd;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	{
		vk::Result res = graphicsQueue_.submit(1, &submitInfo, inFlightFences_[currentFrame_].get());
		if (res != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}
	}

	vk::SwapchainKHR swapChains[] = { swapChain_.get()};
	vk::PresentInfoKHR presentInfo{};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &frame.imageIndex;

	vk::Result res = presentQueue_.presentKHR(presentInfo);

	bool needRecreate =
		framebufferResized_ ||
		res == vk::Result::eErrorOutOfDateKHR ||
		res == vk::Result::eSuboptimalKHR;

	if (needRecreate)
	{
		framebufferResized_ = false;
		recreateSwapChain();
		currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
		return false;
	}

	if (res != vk::Result::eSuccess)
	{
		std::cerr << "vkQueuePresentKHR failed: " << vk::to_string(res) << "\n";
		throw std::runtime_error("failed to present swapchain image!");
	}

	currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
	return true;
} // end of endFrame()

uint32_t VulkanMain::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
{
	vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice_.getMemoryProperties();

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
	vk::DeviceSize size,
	vk::BufferUsageFlags usage,
	vk::MemoryPropertyFlags properties,
	vk::Buffer& buffer,
	vk::DeviceMemory& bufferMemory) const
{
	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;

	{
		auto rv = device_->createBuffer(bufferInfo, nullptr);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create buffer!");
		}
		buffer = rv.value;
	}

	vk::MemoryRequirements memRequirements = device_->getBufferMemoryRequirements(buffer);

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	{
		auto rv = device_->allocateMemory(allocInfo, nullptr);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to allocate buffer memory!");
		}
		bufferMemory = rv.value;
	}

	{
		vk::Result res = device_->bindBufferMemory(buffer, bufferMemory, 0);
		if (res != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to bind buffer memory!");
		}
	}
} // end of createBuffer()

vk::CommandBuffer VulkanMain::beginSingleTimeCommands() const
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandPool = commandPool_.get();
	allocInfo.commandBufferCount = 1;

	vk::CommandBuffer commandBuffer{};
	{
		auto rv = device_->allocateCommandBuffers(allocInfo);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("allocateCommandBuffers failed!");
		}
		commandBuffer = rv.value[0];
	}

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

	if (commandBuffer.begin(beginInfo) != vk::Result::eSuccess)
	{
		throw std::runtime_error("beginSingleTimeCommands: begin failed!");
	}

	return commandBuffer;
} // end of beginSingleTimeCommands()

void VulkanMain::endSingleTimeCommands(vk::CommandBuffer commandBuffer) const
{
	commandBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	graphicsQueue_.submit(1, &submitInfo, nullptr);
	graphicsQueue_.waitIdle();

	device_->freeCommandBuffers(commandPool_.get(), 1, &commandBuffer);
} // end of endSingleTimeCommands()

void VulkanMain::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) const
{
	vk::CommandBuffer commandBuffer = beginSingleTimeCommands();

	vk::BufferCopy copyRegion{};
	copyRegion.size = size;
	commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
} // end of copyBuffer()

vk::Format VulkanMain::getDepthFormat() const
{
	std::vector<vk::Format> candidates =
	{
		vk::Format::eD32Sfloat,
		vk::Format::eD32SfloatS8Uint,
		vk::Format::eD24UnormS8Uint
	};

	for (vk::Format format : candidates)
	{
		auto props = physicalDevice_.getFormatProperties(format);

		if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		{
			return format;
		}
	}

	throw std::runtime_error("Failed to find supported depth format!");
} // end of getDepthFormat()


//--- PRIVATE ---//
void VulkanMain::createInstance()
{
	VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

	// check for validation layers
	if (enableValidationLayers_ && !checkValidationLayerSupport()) 
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	vk::ApplicationInfo appInfo{};
	appInfo.pApplicationName = "Project Atlas";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "Atlas";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_4;

	vk::InstanceCreateInfo createInfo{};
	createInfo.pApplicationInfo = &appInfo;

	std::vector<const char*> extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
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

	{
		auto rv = vk::createInstanceUnique(createInfo, nullptr);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create instance!");
		}
		instance_ = std::move(rv.value);
	}
	VULKAN_HPP_DEFAULT_DISPATCHER.init(instance_.get());
} // end of createInstance()

void VulkanMain::setupDebugMessenger()
{
	if (!enableValidationLayers_) return;

	vk::DebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	{
		auto rv = instance_->createDebugUtilsMessengerEXTUnique(createInfo);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
		debugMessenger_ = std::move(rv.value);
	}
} // end of setupDebugMessenger()

void VulkanMain::createSurface()
{
	VkSurfaceKHR rawSurface{};
	if (glfwCreateWindowSurface(instance_.get(), window_, nullptr, &rawSurface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	surface_ = vk::UniqueSurfaceKHR(vk::SurfaceKHR(rawSurface), instance_.get());
} // end of createSurface()

void VulkanMain::pickPhysicalDevice()
{
	auto rv = instance_->enumeratePhysicalDevices();
	
	if (rv.result != vk::Result::eSuccess || rv.value.empty())
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	for (const auto& device : rv.value) 
	{
		if (isDeviceSuitable(device)) 
		{
			physicalDevice_ = device;
			msaaSamples_ = getMaxUsableSampleCount();
			break;
		}
	} // end for

	if (!physicalDevice_) 
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
} // end of pickPhysicalDevice()

void VulkanMain::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);

	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { 
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) 
	{
		vk::DeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	} // end for

	vk::PhysicalDeviceFeatures2 deviceFeatures2{};
	deviceFeatures2.features.samplerAnisotropy = VK_TRUE;
	deviceFeatures2.features.sampleRateShading = VK_TRUE;
	deviceFeatures2.features.shaderClipDistance = VK_TRUE;

	vk::PhysicalDeviceDynamicRenderingFeatures dynamicRendering{};
	dynamicRendering.dynamicRendering = VK_TRUE;

	deviceFeatures2.pNext = &dynamicRendering;

	vk::DeviceCreateInfo createInfo{};
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
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	auto rv = physicalDevice_.createDeviceUnique(createInfo, nullptr);
	if (rv.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	device_ = std::move(rv.value);

	graphicsQueue_ = device_->getQueue(indices.graphicsFamily.value(), 0);
	presentQueue_ = device_->getQueue(indices.presentFamily.value(), 0);

	VULKAN_HPP_DEFAULT_DISPATCHER.init(device_.get());
} // end of createLogicalDevice()

void VulkanMain::createSwapChain()
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice_);

	vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 &&
		imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo{};
	createInfo.surface = surface_.get();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) 
	{
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else 
	{
		createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = nullptr;

	auto rv = device_->createSwapchainKHRUnique(createInfo, nullptr);
	if (rv.result != vk::Result::eSuccess) 
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	swapChain_ = std::move(rv.value);

	// get swapchain images
	{
		auto rv = device_->getSwapchainImagesKHR(swapChain_.get());
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("getSwapchainImagesKHR failed!");
		}
		swapChainImages_ = std::move(rv.value);
	}

	swapChainLayouts_.assign(swapChainImages_.size(), vk::ImageLayout::eUndefined);
	swapChainImageFormat_ = surfaceFormat.format;
	swapChainExtent_ = extent;
} // end of createSwapChain()

void VulkanMain::createImageViews()
{
	swapChainImageViews_.clear();
	swapChainImageViews_.reserve(swapChainImages_.size());

	for (size_t i = 0; i < swapChainImages_.size(); ++i) 
	{
		vk::ImageViewCreateInfo createInfo{};
		createInfo.image = swapChainImages_[i];
		createInfo.viewType = vk::ImageViewType::e2D;
		createInfo.format = swapChainImageFormat_;
		createInfo.subresourceRange = {
			vk::ImageAspectFlagBits::eColor,
			0, 1,
			0, 1
		};

		auto rv = device_->createImageViewUnique(createInfo, nullptr);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create image view!");
		}

		swapChainImageViews_.push_back(std::move(rv.value));
	} // end for
} // end of createImageViews()

void VulkanMain::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice_);

	vk::CommandPoolCreateInfo poolInfo{};
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	auto rv = device_->createCommandPoolUnique(poolInfo, nullptr);
	if (rv.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create command pool!");
	}

	commandPool_ = std::move(rv.value);
} // end of createCommandPool()

void VulkanMain::createCommandBuffers()
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.commandPool = commandPool_.get();
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

	auto rv = device_->allocateCommandBuffers(allocInfo);
	if (rv.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

	commandBuffers_ = std::move(rv.value);
} // end of createCommandBuffers()

void VulkanMain::createSyncObjects()
{
	imageAvailableSemaphores_.clear();
	inFlightFences_.clear();

	imageAvailableSemaphores_.reserve(MAX_FRAMES_IN_FLIGHT);
	inFlightFences_.reserve(MAX_FRAMES_IN_FLIGHT);

	vk::SemaphoreCreateInfo semInfo{};
	vk::FenceCreateInfo fenceInfo{};
	fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		{
			auto rv = device_->createSemaphoreUnique(semInfo, nullptr);
			if (rv.result != vk::Result::eSuccess)
			{
				throw std::runtime_error("failed to create per-frame semaphore!");
			}

			imageAvailableSemaphores_.push_back(std::move(rv.value));
		}

		{
			auto rv = device_->createFenceUnique(fenceInfo, nullptr);
			if (rv.result != vk::Result::eSuccess)
			{
				throw std::runtime_error("failed to create per-frame fence!");
			}

			inFlightFences_.push_back(std::move(rv.value));
		}
	} // end for

	createPerImageSync();
} // end of createSyncObjects()

void VulkanMain::cleanupSwapChain()
{
	if (!device_) return;

	renderFinishedPerImage_.clear();
	imagesInFlight_.clear();

	swapChainImageViews_.clear();
	swapChain_.reset();

	swapChainImages_.clear();
	swapChainLayouts_.clear();
	swapChainImageFormat_ = vk::Format::eUndefined;
	swapChainExtent_ = vk::Extent2D{ 0u, 0u };
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

	device_->waitIdle();

	cleanupSwapChain();
	createSwapChain();
	createImageViews();
	createPerImageSync();

	framebufferResized_ = false;
} // end of recreateSwapChain()


void VulkanMain::populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = vk::DebugUtilsMessengerCreateInfoEXT{};
	createInfo.messageSeverity =
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

	createInfo.messageType =
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

	createInfo.pfnUserCallback = debugCallback;
} // end of populateDebugMessengerCreateInfo()

bool VulkanMain::checkValidationLayerSupport()
{
	auto rv = vk::enumerateInstanceLayerProperties();
	if (rv.result != vk::Result::eSuccess)
	{
		return false;
	}

	std::vector<vk::LayerProperties> availableLayers = rv.value;

	for (const char* layerName : validationLayers_)
	{
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (std::strcmp(layerName, layerProperties.layerName) == 0)
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
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers_)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
} // end of getRequiredExtensions()

bool VulkanMain::isDeviceSuitable(vk::PhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	vk::PhysicalDeviceDynamicRenderingFeatures dyn{};
	vk::PhysicalDeviceFeatures2 feats2{};
	feats2.pNext = &dyn;

	device.getFeatures2(&feats2);

	return indices.isComplete() 
		&& extensionsSupported
		&& swapChainAdequate 
		&& feats2.features.samplerAnisotropy 
		&& feats2.features.sampleRateShading 
		&& dyn.dynamicRendering;
} // end of isDeviceSuitable()

QueueFamilyIndices VulkanMain::findQueueFamilies(vk::PhysicalDevice device) 
{
	QueueFamilyIndices indices;
	
	auto queueFamilies = device.getQueueFamilyProperties();

	uint32_t i = 0;
	for (const auto& qf : queueFamilies) 
	{
		auto rv = device.getSurfaceSupportKHR(i, surface_.get());
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("getSurfaceSupportKHR failed!");
		}
		
		if (rv.value)
		{
			indices.presentFamily = i;
		}

		if (qf.queueFlags & vk::QueueFlagBits::eGraphics)
		{
			indices.graphicsFamily = i;
		}

		if (indices.isComplete())
			break;

		++i;
	} // end for
	return indices;
} // end of findQueueFamilies()

vk::SampleCountFlagBits VulkanMain::getMaxUsableSampleCount() 
{
	vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice_.getProperties();

	vk::SampleCountFlags counts = 
		physicalDeviceProperties.limits.framebufferColorSampleCounts &
		physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	if (counts & vk::SampleCountFlagBits::e64) return vk::SampleCountFlagBits::e64;
	if (counts & vk::SampleCountFlagBits::e32) return vk::SampleCountFlagBits::e32;
	if (counts & vk::SampleCountFlagBits::e16) return vk::SampleCountFlagBits::e16;
	if (counts & vk::SampleCountFlagBits::e8)  return vk::SampleCountFlagBits::e8;
	if (counts & vk::SampleCountFlagBits::e4)  return vk::SampleCountFlagBits::e4;
	if (counts & vk::SampleCountFlagBits::e2)  return vk::SampleCountFlagBits::e2;

	return vk::SampleCountFlagBits::e1;
} // end of getMaxUsableSampleCount()

bool VulkanMain::checkDeviceExtensionSupport(vk::PhysicalDevice device) 
{
	auto rv = device.enumerateDeviceExtensionProperties(nullptr);
	if (rv.result != vk::Result::eSuccess)
	{
		return false;
	}

	std::set<std::string> requiredExtensions(deviceExtensions_.begin(), deviceExtensions_.end());
	for (const auto& ext : rv.value) 
	{
		requiredExtensions.erase(ext.extensionName);
	} // end for

	return requiredExtensions.empty();
} // end of checkDeviceExtensionSupport()

SwapChainSupportDetails VulkanMain::querySwapChainSupport(vk::PhysicalDevice device) 
{
	SwapChainSupportDetails details;
	
	{
		auto rv = device.getSurfaceCapabilitiesKHR(surface_.get());
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("getSurfaceCapabilitiesKHR failed!");
		}

		details.capabilities = rv.value;
	}

	{
		auto rv = device.getSurfaceFormatsKHR(surface_.get());
		if (rv.result == vk::Result::eSuccess)
		{
			details.formats = std::move(rv.value);
		}
	}

	{
		auto rv = device.getSurfacePresentModesKHR(surface_.get());
		if (rv.result == vk::Result::eSuccess)
		{
			details.presentModes = std::move(rv.value);
		}
	}

	return details;
} // end of querySwapChainSupport()

vk::UniqueImageView VulkanMain::createImageView(vk::Image image, vk::Format format,
	vk::ImageAspectFlags aspectFlags, uint32_t mipLevels) 
{
	vk::ImageViewCreateInfo viewInfo{};
	viewInfo.image = image;
	viewInfo.viewType = vk::ImageViewType::e2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	auto rv = device_->createImageViewUnique(viewInfo, nullptr);
	if (rv.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("failed to create image view!");
	}

	return std::move(rv.value);
} // end of createImageView()

vk::SurfaceFormatKHR VulkanMain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) 
	{
		if (availableFormat.format == vk::Format::eB8G8R8A8Srgb &&
			availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) 
		{
			return availableFormat;
		}
	} // end for

	return availableFormats[0];
} // end of chooseSwapSurfaceFormat()

vk::PresentModeKHR VulkanMain::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
	for (const auto& m : availablePresentModes) 
	{
		if (m == vk::PresentModeKHR::eMailbox) 
		{
			return m;
		}
	} // end for
	return vk::PresentModeKHR::eFifo;
} // end of chooseSwapPresentMode()

vk::Extent2D VulkanMain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilites)
{
	if (capabilites.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
	{
		return capabilites.currentExtent;
	}

	int width;
	int height;
	glfwGetFramebufferSize(window_, &width, &height);

	vk::Extent2D actualExtent = {
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
} // end of chooseSwapExtent()

void VulkanMain::createPerImageSync()
{
	// per-swapchain image sync
	renderFinishedPerImage_.clear();
	renderFinishedPerImage_.reserve(swapChainImages_.size());

	imagesInFlight_.assign(swapChainImages_.size(), vk::Fence{});

	vk::SemaphoreCreateInfo semInfo{};

	for (uint32_t i = 0; i < swapChainImages_.size(); ++i)
	{
		auto rv = device_->createSemaphoreUnique(semInfo, nullptr);
		if (rv.result != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to create per-image render finished semaphore!");
		}
		renderFinishedPerImage_.push_back(std::move(rv.value));
	} // end for
} // end of createPerImageSync()