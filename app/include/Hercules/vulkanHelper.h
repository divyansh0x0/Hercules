#pragma once
#include<string>
#include<vulkan/vulkan_core.h>
std::string getTranslatedErrorCode(VkResult vkResult){
    switch (vkResult)
    {
    case  VK_TIMEOUT: 
        return "Timed out";
    case  VK_EVENT_RESET:
        return "Event was reset";

    case  VK_INCOMPLETE:
        return "Incomplete";
    case  VK_ERROR_OUT_OF_HOST_MEMORY:
        return "Out of host(cpu) memory";
    case  VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "Out of device (gpu) memory";
    case  VK_ERROR_INITIALIZATION_FAILED:
        return "Inititialization failed";
    case  VK_ERROR_DEVICE_LOST:
        return "Device lost";
    case  VK_ERROR_MEMORY_MAP_FAILED:
        return "Memory map failed";
    case  VK_ERROR_LAYER_NOT_PRESENT:
        return "Requested layer is not present";
    case  VK_ERROR_EXTENSION_NOT_PRESENT:
        return "Requested extension is not present on device";
    case  VK_ERROR_FEATURE_NOT_PRESENT:
        return "Requested feature is not availabled";
    default:
        return "Cant translate error code " + std::to_string(vkResult);
    }

}