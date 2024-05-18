#include <Hercules/shader.h>
#include <fstream>
// #include <filesystem>
VkShaderModule CreateShaderModule(const std::string &filename, VkDevice device)
{
    // auto path = std::filesystem::current_path() / "shaders"/ filename;
    // Read shader data
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open shader file: " + filename);
    }
    size_t code_size = (size_t)file.tellg();
    uint32_t *buffer = new uint32_t[code_size];
    file.seekg(0);
    file.read(reinterpret_cast<char *>(buffer), code_size);
    file.close();

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code_size;
    createInfo.pCode = buffer;
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        delete[] buffer;
        throw std::runtime_error("Failed to create shader module!");
    }
    delete[] buffer;

    return shaderModule;
}