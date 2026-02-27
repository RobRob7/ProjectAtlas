#ifndef SHADER_MODULE_VK_H
#define SHADER_MODULE_VK_H

#include <vulkan/vulkan.h>

#include <string_view>
#include <vector>
#include <cstdint>
#include <filesystem>

class ShaderModuleVk
{
public:
	ShaderModuleVk(VkDevice device, std::string_view vertexPathFile, std::string_view fragPathFile);
	~ShaderModuleVk() noexcept;

	ShaderModuleVk(const ShaderModuleVk&) = delete;
	ShaderModuleVk& operator=(const ShaderModuleVk&) = delete;

	ShaderModuleVk(ShaderModuleVk&& other) noexcept;
	ShaderModuleVk& operator=(ShaderModuleVk&& other) noexcept;

	VkShaderModule vert() const { return vertShaderModule_; }
	VkShaderModule frag() const { return fragShaderModule_; }

private:
	void destroy() noexcept;
	VkShaderModule createShaderModule(const std::vector<uint32_t>& code);
	std::vector<uint32_t> readFile(const std::filesystem::path& pathFile);
private:
	VkDevice device_{ VK_NULL_HANDLE };
	VkShaderModule vertShaderModule_{ VK_NULL_HANDLE };
	VkShaderModule fragShaderModule_{ VK_NULL_HANDLE };
};

#endif
