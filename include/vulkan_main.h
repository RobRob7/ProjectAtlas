#ifndef VULKAN_MAIN_H
#define VULKAN_MAIN_H

#include <vulkan/vulkan.hpp>

#include <vector>
#include <cstdint>
#include <optional>

struct GLFWwindow;

struct FrameContext
{
    vk::CommandBuffer cmd{};

    uint32_t frameIndex = 0;
    uint32_t imageIndex = 0;

    vk::Image depthImage{};
    vk::ImageView depthImageView{};

    vk::Image swapchainImage{};
    vk::ImageView swapchainImageView{};

    vk::Extent2D extent{};
    vk::Format format{ vk::Format::eUndefined };
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() const 
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    } // end of isComplete()
};

struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities{};
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class VulkanMain
{
public:
    explicit VulkanMain(GLFWwindow* window);
    ~VulkanMain();

    void init();
    void waitIdle() const;

    bool beginFrame(FrameContext& out);
    bool endFrame(const FrameContext& frame);

    void notifyFramebufferResized() { framebufferResized_ = true; }

    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

    void createBuffer(
        vk::DeviceSize size,
        vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties,
        vk::Buffer& buffer,
        vk::DeviceMemory& bufferMemory) const;

    vk::CommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(vk::CommandBuffer commandBuffer) const;

    void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) const;

    vk::Format findDepthFormat() const;
    vk::Format getDepthFormat() const { return depthFormat_; }

    vk::Device getDevice() const { return device_.get(); }
    vk::PhysicalDevice getPhysicalDevice() const { return physicalDevice_; }
    vk::Queue getGraphicsQueue() const { return graphicsQueue_; }
    vk::Queue getPresentQueue() const { return presentQueue_; }
    vk::CommandPool getCommandPool() const { return commandPool_.get(); }

    vk::Format getSwapChainImageFormat() const { return swapChainImageFormat_; }
    vk::Extent2D getSwapChainExtent() const { return swapChainExtent_; }
    vk::ImageView getSwapChainImageView(size_t i) const { return swapChainImageViews_[i].get(); }
    size_t getSwapChainImageViewCount() const { return swapChainImageViews_.size(); }
    const std::vector<vk::Image>& getSwapChainImages() const { return swapChainImages_; }

    vk::SwapchainKHR getSwapChain() const { return swapChain_.get(); }
    vk::SurfaceKHR getSurface() const { return surface_.get(); }
    vk::Instance getInstance() const { return instance_.get(); }

    vk::ImageLayout getSwapChainLayout(uint32_t imageIndex) const { return swapChainLayouts_[imageIndex]; }
    void setSwapChainLayout(uint32_t imageIndex, vk::ImageLayout layout) { swapChainLayouts_[imageIndex] = layout; }

    uint32_t getMaxFramesInFlight() const { return MAX_FRAMES_IN_FLIGHT; }

private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createCommandPool();

    void createCommandBuffers();
    void createSyncObjects();

    void cleanupSwapChain();
    void recreateSwapChain();

    void populateDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);

    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    bool isDeviceSuitable(vk::PhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
    vk::SampleCountFlagBits getMaxUsableSampleCount();
    bool checkDeviceExtensionSupport(vk::PhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device);

    vk::UniqueImageView createImageView(vk::Image image, vk::Format format,
        vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);

    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilites);

    void createPerImageSync();

private:
    const std::vector<const char*> validationLayers_ = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions_ = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    bool enableValidationLayers_{ false };
    bool initialized_{ false };

    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    bool framebufferResized_{ false };
    uint32_t currentFrame_ = 0;

    vk::SampleCountFlagBits msaaSamples_{ vk::SampleCountFlagBits::e1 };

    GLFWwindow* window_{};

    vk::UniqueInstance instance_{};
    vk::UniqueDebugUtilsMessengerEXT debugMessenger_{};
    vk::UniqueSurfaceKHR surface_{};

    vk::PhysicalDevice physicalDevice_{};
    vk::UniqueDevice device_{};

    vk::Queue graphicsQueue_{};
    vk::Queue presentQueue_{};

    vk::UniqueImage depthImage_{};
    vk::UniqueDeviceMemory depthImageMemory_{};
    vk::UniqueImageView depthImageView_{};
    vk::Format depthFormat_{ vk::Format::eUndefined };
    vk::ImageLayout depthImageLayout_{ vk::ImageLayout::eUndefined };

    vk::UniqueSwapchainKHR swapChain_{};
    std::vector<vk::Image> swapChainImages_;
    vk::Format swapChainImageFormat_{ vk::Format::eUndefined };
    vk::Extent2D swapChainExtent_{};
    std::vector<vk::UniqueImageView> swapChainImageViews_;
    std::vector<vk::ImageLayout> swapChainLayouts_;

    vk::UniqueCommandPool commandPool_{};

    std::vector<vk::CommandBuffer> commandBuffers_;
    std::vector<vk::UniqueSemaphore> imageAvailableSemaphores_;
    std::vector<vk::UniqueFence> inFlightFences_;

    std::vector<vk::UniqueSemaphore> renderFinishedPerImage_;
    std::vector<vk::Fence> imagesInFlight_;
};

#endif