#include "texture_cubemap_vk.h"

#include "buffer_vk.h"
#include "vulkan_main.h"

#include <stb/stb_image.h>

#include <vector>
#include <string_view>
#include <array>
#include <stdexcept>
#include <string>

namespace
{
	void transitionImageLayout(
		VulkanMain& vk,
		vk::Image image,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		uint32_t layers
	)
	{
		vk::CommandBuffer cmd = vk.beginSingleTimeCommands();

		vk::ImageMemoryBarrier barrier{};
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
		barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = layers;

		vk::PipelineStageFlags srcStage;
		vk::PipelineStageFlags dstStage;

		if (oldLayout == vk::ImageLayout::eUndefined &&
			newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
			dstStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
			newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			srcStage = vk::PipelineStageFlagBits::eTransfer;
			dstStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else
		{
			throw std::runtime_error("Unsupported cubemap layout transition");
		}

		cmd.pipelineBarrier(srcStage, dstStage, {}, {}, {}, barrier);
		vk.endSingleTimeCommands(cmd);
	}

	void copyBufferToImage(
		VulkanMain& vk,
		vk::Buffer buffer,
		vk::Image image,
		uint32_t width,
		uint32_t height,
		uint32_t layers
	)
	{
		vk::CommandBuffer cmd = vk.beginSingleTimeCommands();

		std::vector<vk::BufferImageCopy> regions;
		regions.reserve(layers);

		const vk::DeviceSize layerSize = static_cast<vk::DeviceSize>(width) * height * 4;

		for (uint32_t layer = 0; layer < layers; ++layer)
		{
			vk::BufferImageCopy region{};
			region.bufferOffset = layer * layerSize;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = layer;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = vk::Offset3D{ 0, 0, 0 };
			region.imageExtent = vk::Extent3D{ width, height, 1 };
			regions.push_back(region);
		}

		cmd.copyBufferToImage(
			buffer,
			image,
			vk::ImageLayout::eTransferDstOptimal,
			static_cast<uint32_t>(regions.size()),
			regions.data()
		);

		vk.endSingleTimeCommands(cmd);
	}
}

//--- PUBLIC ---//
TextureCubemapVk::TextureCubemapVk(VulkanMain& vk)
	: vk_(vk), image_(vk)
{
} // end of constructor

TextureCubemapVk::~TextureCubemapVk() = default;

void TextureCubemapVk::loadFromFiles(const std::array<std::string_view, 6>& faces)
{
	int texWidth = 0;
	int texHeight = 0;
	int texChannels = 0;

	std::vector<stbi_uc*> loadedFaces;
	loadedFaces.reserve(6);

	for (size_t i = 0; i < 6; ++i)
	{
		int w = 0;
		int h = 0;
		int c = 0;

		std::string pathToTexture = std::string(RESOURCES_PATH) + std::string(faces[i]);
		stbi_uc* pixels = stbi_load(pathToTexture.data(), &w, &h, &c, STBI_rgb_alpha);
		if (!pixels)
		{
			for (stbi_uc* p : loadedFaces) stbi_image_free(p);
			throw std::runtime_error("TextureCubemapVk::loadFromFiles - failed to load face");
		}

		if (i == 0)
		{
			texWidth = w;
			texHeight = h;
			texChannels = c;
		}
		else if (w != texWidth || h != texHeight)
		{
			for (stbi_uc* p : loadedFaces) stbi_image_free(p);
			stbi_image_free(pixels);
			throw std::runtime_error("TextureCubemapVk::loadFromFiles - cubemap faces must match dimensions");
		}

		loadedFaces.push_back(pixels);
	}

	const vk::DeviceSize layerSize = static_cast<vk::DeviceSize>(texWidth) * texHeight * 4;
	const vk::DeviceSize totalSize = layerSize * 6;

	std::vector<unsigned char> packed(static_cast<size_t>(totalSize));
	for (size_t i = 0; i < 6; ++i)
	{
		std::memcpy(packed.data() + i * static_cast<size_t>(layerSize), loadedFaces[i], static_cast<size_t>(layerSize));
		stbi_image_free(loadedFaces[i]);
	}

	BufferVk staging(vk_);
	staging.create(
		totalSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	staging.upload(packed.data(), totalSize);

	image_.createImage(
		static_cast<uint32_t>(texWidth),
		static_cast<uint32_t>(texHeight),
		6,
		vk::SampleCountFlagBits::e1,
		vk::Format::eR8G8B8A8Srgb,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal,
		vk::ImageCreateFlagBits::eCubeCompatible
	);

	transitionImageLayout(vk_, image_.image(), vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 6);
	copyBufferToImage(vk_, staging.getBuffer(), image_.image(), texWidth, texHeight, 6);
	transitionImageLayout(vk_, image_.image(), vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 6);

	image_.createImageView(
		vk::Format::eR8G8B8A8Srgb,
		vk::ImageAspectFlagBits::eColor,
		vk::ImageViewType::eCube,
		6
	);

	image_.createSampler(
		vk::Filter::eLinear,
		vk::Filter::eLinear,
		vk::SamplerAddressMode::eClampToEdge
	);
} // end of loadFromFiles()


//--- PRIVATE ---//
void TextureCubemapVk::destroy()
{
	image_.destroy();
} // end of destroy()