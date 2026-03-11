#ifndef IMAGE_VK_H
#define IMAGE_VK_H

#include <vulkan/vulkan.hpp>

#include <cstdint>

class VulkanMain;

class ImageVk
{
public:
    explicit ImageVk(VulkanMain& vk);
    ~ImageVk();

    ImageVk(const ImageVk&) = delete;
    ImageVk& operator=(const ImageVk&) = delete;

    ImageVk(ImageVk&&) noexcept = default;
    ImageVk& operator=(ImageVk&&) noexcept = default;

    void createImage(
        uint32_t width,
        uint32_t height,
        uint32_t layers,
        bool autoMipLevels,
        vk::SampleCountFlagBits samples,
        vk::Format format,
        vk::ImageTiling tiling,
        vk::ImageUsageFlags usage,
        vk::MemoryPropertyFlags properties,
        vk::ImageCreateFlags flags = {}
    );

    void generateMipmaps(
        vk::Image image,
        vk::Format imageFormat,
        int32_t texWidth,
        int32_t texHeight,
        uint32_t mipLevels,
        uint32_t layers = 1
    );

    void createImageView(
        vk::Format format,
        vk::ImageAspectFlags aspectFlags,
        vk::ImageViewType viewType,
        uint32_t layers
    );

    void createSampler(
        vk::Filter magFilter = vk::Filter::eLinear,
        vk::Filter minFilter = vk::Filter::eLinear,
        vk::SamplerAddressMode addressMode = vk::SamplerAddressMode::eRepeat
    );

	void destroy();

    bool valid() const { return static_cast<bool>(image_); }

    vk::Image image() const { return image_.get(); }
    vk::DeviceMemory memory() const { return memory_.get(); }
    vk::ImageView view() const { return view_.get(); }
    vk::Sampler sampler() const { return sampler_.get(); }

    vk::Format format() const { return format_; }
    uint32_t width() const { return width_; }
    uint32_t height() const { return height_; }
    uint32_t layers() const { return layers_; }
    uint32_t mipLevels() const { return mipLevels_; }

private:
    VulkanMain& vk_;

    vk::UniqueImage image_{};
    vk::UniqueDeviceMemory memory_{};
    vk::UniqueImageView view_{};
    vk::UniqueSampler sampler_{};

    vk::Format format_{ vk::Format::eUndefined };
    uint32_t width_{ 0 };
    uint32_t height_{ 0 };
    uint32_t layers_{ 0 };

    uint32_t mipLevels_{ 1 };

};

#endif
