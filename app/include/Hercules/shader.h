#pragma once
#include <string>
#include<vulkan/vulkan_core.h>

VkShaderModule CreateShaderModule(const std::string &filename, VkDevice device);