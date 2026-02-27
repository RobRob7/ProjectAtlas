#include "shader_vk.h"

#include <stdexcept>
#include <fstream>
//--- PUBLIC ---//
ShaderModuleVk::ShaderModuleVk(VkDevice device, std::string_view vertexPathFile, std::string_view fragPathFile)
	: device_(device)
{
	if (device_ == VK_NULL_HANDLE)
	{
		throw std::runtime_error("ShaderModuleVk: device is VK_NULL_HANDLE");
	}

	vertShaderModule_ = createShaderModule(readFile(std::filesystem::path(RESOURCES_PATH) / vertexPathFile));
	fragShaderModule_ = createShaderModule(readFile(std::filesystem::path(RESOURCES_PATH) / fragPathFile));
} // end of constructor

ShaderModuleVk::~ShaderModuleVk() noexcept
{
	destroy();
} // end of destructor

ShaderModuleVk::ShaderModuleVk(ShaderModuleVk&& other) noexcept
{
	device_ = other.device_;
	vertShaderModule_ = other.vertShaderModule_;
	fragShaderModule_ = other.fragShaderModule_;

	other.device_ = VK_NULL_HANDLE;
	other.vertShaderModule_ = VK_NULL_HANDLE;
	other.fragShaderModule_ = VK_NULL_HANDLE;
} // end of move constructor

ShaderModuleVk& ShaderModuleVk::operator=(ShaderModuleVk&& other) noexcept
{
	if (this == &other) return *this;

	destroy();

	device_ = other.device_;
	vertShaderModule_ = other.vertShaderModule_;
	fragShaderModule_ = other.fragShaderModule_;

	other.device_ = VK_NULL_HANDLE;
	other.vertShaderModule_ = VK_NULL_HANDLE;
	other.fragShaderModule_ = VK_NULL_HANDLE;

	return *this;
} // end of move assignment


//--- PRIVATE ---//
void ShaderModuleVk::destroy() noexcept
{
	if (device_ == VK_NULL_HANDLE) return;

	if (vertShaderModule_ != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(device_, vertShaderModule_, nullptr);
		vertShaderModule_ = VK_NULL_HANDLE;
	}
	if (fragShaderModule_ != VK_NULL_HANDLE)
	{
		vkDestroyShaderModule(device_, fragShaderModule_, nullptr);
		fragShaderModule_ = VK_NULL_HANDLE;
	}
} // end of destroy()

VkShaderModule ShaderModuleVk::createShaderModule(const std::vector<uint32_t>& code)
{
	if (code.empty())
	{
		throw std::runtime_error("SPIR-V code is empty");
	}

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size() * sizeof(uint32_t);
	createInfo.pCode = code.data();

	VkShaderModule shaderModule{ VK_NULL_HANDLE };
	if (vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
} // end of createShaderModule()

std::vector<uint32_t> ShaderModuleVk::readFile(const std::filesystem::path& pathFile)
{
	std::ifstream file(pathFile, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file: " + pathFile.string());
	}

	const std::streamsize fileSize = file.tellg();
	if (fileSize <= 0)
	{
		throw std::runtime_error("shader file is empty (or tellg failed)");
	}

	if ((fileSize % 4) != 0)
	{
		throw std::runtime_error("SPIR-V file size is not a multiple of 4 bytes!");
	}

	std::vector<uint32_t> buffer(static_cast<size_t>(fileSize / 4));

	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
	file.close();

	return buffer;
} // end of readFile()