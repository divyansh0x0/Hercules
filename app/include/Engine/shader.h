#pragma once
#include <vector>
#include <string>
#include<vulkan_core.h>
std::vector<char> ReadShaderFile(const std::string &filename);
VkShaderModule CreateShaderModule(const std::vector<char>& code, VkDevice device);