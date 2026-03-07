#include "image_vk.h"

#include "vulkan_main.h"

#include <vulkan/vulkan.hpp>

#include <stdexcept>

//--- PUBLIC ---//
ImageVk::ImageVk(VulkanMain& vk)
    : vk_(vk)
{
} // end of constructor

ImageVk::~ImageVk() = default;

void ImageVk::createImage(
    uint32_t width,
    uint32_t height,
    uint32_t layers,
    vk::SampleCountFlagBits samples,
    vk::Format format,
    vk::ImageTiling tiling,
    vk::ImageUsageFlags usage,
    vk::MemoryPropertyFlags properties,
    vk::ImageCreateFlags flags
)
{
	destroy();

	if (width == 0 || height == 0 || layers == 0)
	{
		throw std::runtime_error("ImageVk::createImage - invalid dimensions/layers");
	}

	width_ = width;
	height_ = height;
	layers_ = layers;
	format_ = format;

	vk::Device device = vk_.getDevice();

	vk::ImageCreateInfo ici{};
	ici.flags = flags;
	ici.imageType = vk::ImageType::e2D;
	ici.extent.width = width;
	ici.extent.height = height;
	ici.extent.depth = 1;
	ici.mipLevels = 1;
	ici.arrayLayers = layers;
	ici.format = format;
	ici.tiling = tiling;
	ici.initialLayout = vk::ImageLayout::eUndefined;
	ici.usage = usage;
	ici.samples = samples;
	ici.sharingMode = vk::SharingMode::eExclusive;

	vk::ResultValue rvImage = device.createImageUnique(ici);
	if (rvImage.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("ImageVk::createImage - createImageUnique failed: " + vk::to_string(rvImage.result));
	}
	image_ = std::move(rvImage.value);

	vk::MemoryRequirements memReq = device.getImageMemoryRequirements(image_.get());

	vk::MemoryAllocateInfo mai{};
	mai.allocationSize = memReq.size;
	mai.memoryTypeIndex = vk_.findMemoryType(memReq.memoryTypeBits, properties);

	vk::ResultValue rvMem = device.allocateMemoryUnique(mai);
	if (rvMem.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("ImageVk::createImage - allocateMemoryUnique failed: " + vk::to_string(rvMem.result));
	}
	memory_ = std::move(rvMem.value);

	vk::Result bindRes = device.bindImageMemory(image_.get(), memory_.get(), 0);
	if (bindRes != vk::Result::eSuccess)
	{
		throw std::runtime_error("ImageVk::createImage - bindImageMemory failed: " + vk::to_string(bindRes));
	}
} // end of createImage()

void ImageVk::createImageView(
    vk::Format format,
    vk::ImageAspectFlags aspectFlags,
    vk::ImageViewType viewType,
    uint32_t layers
)
{
	if (!image_)
	{
		throw std::runtime_error("ImageVk::createImageView - image not created");
	}

	vk::ImageViewCreateInfo ivci{};
	ivci.image = image_.get();
	ivci.viewType = viewType;
	ivci.format = format;
	ivci.subresourceRange.aspectMask = aspectFlags;
	ivci.subresourceRange.baseMipLevel = 0;
	ivci.subresourceRange.levelCount = 1;
	ivci.subresourceRange.baseArrayLayer = 0;
	ivci.subresourceRange.layerCount = layers;

	vk::ResultValue rv = vk_.getDevice().createImageViewUnique(ivci);
	if (rv.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("ImageVk::createImageView - createImageViewUnique failed: " + vk::to_string(rv.result));
	}
	view_ = std::move(rv.value);
} // end of createImageView()

void ImageVk::createSampler(
    vk::Filter magFilter,
    vk::Filter minFilter,
    vk::SamplerAddressMode addressMode
)
{
	vk::SamplerCreateInfo sci{};
	sci.magFilter = magFilter;
	sci.minFilter = minFilter;
	sci.addressModeU = addressMode;
	sci.addressModeV = addressMode;
	sci.addressModeW = addressMode;
	sci.anisotropyEnable = vk::True;
	sci.maxAnisotropy = 16.0f;
	sci.borderColor = vk::BorderColor::eIntOpaqueBlack;
	sci.unnormalizedCoordinates = vk::False;
	sci.compareEnable = vk::False;
	sci.compareOp = vk::CompareOp::eAlways;
	sci.mipmapMode = vk::SamplerMipmapMode::eLinear;
	sci.minLod = 0.0f;
	sci.maxLod = 0.0f;
	sci.mipLodBias = 0.0f;

	vk::ResultValue rv = vk_.getDevice().createSamplerUnique(sci);
	if (rv.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("ImageVk::createSampler - createSamplerUnique failed: " + vk::to_string(rv.result));
	}
	sampler_ = std::move(rv.value);
} // end of createSampler()

void ImageVk::destroy()
{
	sampler_.reset();
	view_.reset();
	image_.reset();
	memory_.reset();

	format_ = vk::Format::eUndefined;
	width_ = 0;
	height_ = 0;
	layers_ = 0;
} // end of destroy()