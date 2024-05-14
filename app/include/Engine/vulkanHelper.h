#pragma once
#include<string>
#include<vulkan/vulkan_core.h>
std::string getTranslatedErrorCode(VkResult vkResult){
    switch (vkResult)
    {
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "Out of host memory";
    
    default:
        return "Cant translate error code " + std::to_string(vkResult);
    }

}