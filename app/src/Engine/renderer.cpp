#include "Engine/renderer.h"
#include "Engine/vulkanHelper.h"
#include <SDL2/SDL_vulkan.h>
#include "logger.h"
#include <string>
#include <vector>
// #include <vulkan/vulkan_raii.hpp>
// #ifdef NDEBUG
// const bool enableValidationLayers = false;
// #else
const bool enableValidationLayers = true;
// #endif
// #define

// #ifdef _WIN32
// // #pragma comment(linker, "/subsystem:windows")
// #define VK_USE_PLATFORM_WIN32_KHR
// #define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
// #endif

#define ARRAY_LENGTH(arr) sizeof(arr) / sizeof(arr[0])
const VkApplicationInfo kAapplicationInfo = {
    VK_STRUCTURE_TYPE_APPLICATION_INFO,
    nullptr,
    "Physics Engine",
    VK_MAKE_VERSION(1, 0, 0),
    "Hercules",
    VK_MAKE_VERSION(1, 0, 0),
    VK_API_VERSION_1_0

};
const char *kValidationLayers[] = {"VK_LAYER_KHRONOS_validation"};
bool GPUHasAllFeatures(VkPhysicalDevice gpu)
{
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(gpu, &deviceFeatures);
    return deviceFeatures.geometryShader;
}
struct QueueFamilyInfo
{
    bool are_all_queues_available;
    uint32_t graphics_family_index;
    uint32_t present_family_index;
    uint32_t queue_count;
};
///-----------------------------------------------------------------------------------------
//                                          Utility methods
//------------------------------------------------------------------------------------------
bool checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (const char *layerName : kValidationLayers)
    {
        bool layerFound = false;

        logger::info("DEBUG LAYERS FOUND:");
        for (const VkLayerProperties layerProperties : availableLayers)
        {
            logger::info("\t" + std::string(layerProperties.layerName));
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}
VkPhysicalDevice *GetSuitableGPUs(VkInstance vulkan_instance)
{
    
    uint32_t physical_device_count;
    vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, nullptr);
    VkPhysicalDevice physical_devices[physical_device_count];
    vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, physical_devices);

    std::vector<VkPhysicalDevice> suitableDevices;
    if (physical_device_count == 0)
    {
        logger::error("Failed to find GPUs with Vulkan support!");
        return nullptr;
    }
    else
    {
        std::string device_type_names[] = {"VK_PHYSICAL_DEVICE_TYPE_OTHER",
                                           "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU",
                                           "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU",
                                           "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU",
                                           "VK_PHYSICAL_DEVICE_TYPE_CPU"};

        logger::info("Following graphics cards were found:");
        VkPhysicalDeviceProperties device_properties;
        for (VkPhysicalDevice device : physical_devices)
        {
            vkGetPhysicalDeviceProperties(device, &device_properties);

            logger::info("\t DEVICE NAME:" + std::string(device_properties.deviceName) +
                         " | DEVICE TYPE:" + device_type_names[device_properties.deviceType] +
                         "| API VERSION:" + std::to_string(device_properties.apiVersion));
            if ((device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) && GPUHasAllFeatures(device))
                suitableDevices.push_back(device);
        }
    }
    return suitableDevices.data();
}

QueueFamilyInfo getQueueFamilyInfo(VkPhysicalDevice gpu, VkInstance vulkan_instance)
{
    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, nullptr);
    VkQueueFamilyProperties queueFamilies[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, queueFamilies);

    // VkSurfaceKHR surface;
    QueueFamilyInfo queue_family_info{};
    int i = 0;
    for (const VkQueueFamilyProperties queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            VkBool32 is_present_supportted = false;
            // vkGetPhysicalDeviceSurfaceSupportKHR()
            queue_family_info.are_all_queues_available = is_present_supportted;
            queue_family_info.queue_count++;
            queue_family_info.graphics_family_index = i;
        }

        i++;
    }
    return queue_family_info;
}

///-----------------------------------------------------------------------------------------
//                                          Public methods
//------------------------------------------------------------------------------------------

Renderer::Renderer()
{
    logger::success("Renderer object created");
}
void Renderer::set_window(SDL_Window* window){
    this->window_ = window;
    this->sdl_renderer_ = SDL_CreateRenderer(window,-1,0);
    if(sdl_renderer_ == nullptr)
        logger::error(SDL_GetError());

}
bool Renderer::Initialize(bool enable_debug_logs)
{
    if(window_ == nullptr){
        logger::error("Call set_window() before initialization of renderer");
        return false;
    }

    // initialize vulkanInstance variable
    logger::info("INITIALIZING RENDERER");
    unsigned int extension_count;
    SDL_Vulkan_GetInstanceExtensions(window_, &extension_count, nullptr);
    const char *extension_names[extension_count];

    SDL_bool b1 = SDL_Vulkan_GetInstanceExtensions(window_, &extension_count, extension_names);
    if(b1)
        logger::success("Extension created successfully");
    if (extension_count == 0)
    {
        logger::error("EXTENSION COUNT IS ZERO");
    }
    else
    {
        logger::info("Available vulkan extensions:");

        for (const char *extension : extension_names)
        {
            logger::info("\t" + std::string(extension));
        }
    }

    const char **enabled_layer_names = nullptr;
    uint32_t enabled_layer_count = 0;
    if (enable_debug_logs)
    {
        if (!checkValidationLayerSupport())
        {
            logger::error("validation layers requested because debugging is enabled, but not available!");
            return false;
        }
        enabled_layer_names = kValidationLayers;
        enabled_layer_count = ARRAY_LENGTH(kValidationLayers);
    }
    VkInstanceCreateInfo vulkan_instance_info = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, // sType
        nullptr,                                // pNext
        0,                                      // flags
        &kAapplicationInfo,                     // pApplicationInfo
        enabled_layer_count,                    // enabledLayerCount
        enabled_layer_names,                    // ppEnabledLayerNames
        extension_count,                        // enabledExtensionCount
        extension_names                         // ppEnabledExtensionNames
    };
    VkResult instance_creation_code = vkCreateInstance(&vulkan_instance_info, nullptr, &vulkan_instance_);
    if (instance_creation_code != VK_SUCCESS)
    {
        logger::error("VULKAN INSTANCE CREATION FAILED! : " + getTranslatedErrorCode(instance_creation_code));
        return false;
    }
    // Finding a gpu with vulkan support
    VkPhysicalDevice *gpus = GetSuitableGPUs(vulkan_instance_);
    if (gpus == nullptr)
        return false;

    VkPhysicalDevice suitableGPU = gpus[0]; //chosing first gpu as suitable gpus
    // vk::PhysicalDevice physicalDevice(suitableGPU)

    const float queue_priority_level = 1.0f;
    QueueFamilyInfo queue_family_info = getQueueFamilyInfo(suitableGPU, vulkan_instance_);
    VkDeviceQueueCreateInfo queue_create_info = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
        nullptr,                                    // pNext
        0,                                          // flags
        queue_family_info.graphics_family_index,    // queueFamilyIndex
        queue_family_info.queue_count,              // queueCount
        &queue_priority_level                       // pQueuePriorities
    };

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo device_create_info{
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, // sType
        nullptr,                              // pNext
        0,                                    // flags
        1,                                    // queueCreateInfoCount
        &queue_create_info,                   // pQueueCreateInfos
        enabled_layer_count,                  // enabledLayerCount
        enabled_layer_names,                  // ppEnabledLayerNames
        extension_count,                      // enabledExtensionCount
        extension_names,                      // ppEnabledExtensionNames
        &deviceFeatures,                      // pEnabledFeatures
    };

    // creates a device and stores it in global gpu_
    VkResult device_creation_result = vkCreateDevice(suitableGPU, &device_create_info, nullptr, &gpu_);
    if (device_creation_result != VK_SUCCESS)
    {
        for(const char* extension_name: extension_names)
            logger::error(std::string(extension_name));
        logger::error("DEVICE CREATION FAILED ERRORCODE: " + std::to_string(device_creation_result) + " | " + getTranslatedErrorCode(device_creation_result));
        return false;
    }
    logger::success("DEVICE CREATED SUCCESSFULLY");


    VkQueue graphics_queue;
    vkGetDeviceQueue(gpu_, queue_family_info.graphics_family_index, 0, &graphics_queue);

    // VkQueue presentQueue;
    // vkGetDeviceQueue(gpu_, presentQueueIndex, 0, &presentQueue);

    // logger::success("RENDERER WAS INITIALIZED SUCCESSFULLY");

    // SDL_Log("Initialized with errors: %s", SDL_GetError());
    return true;
}
SDL_Rect rect = {0, 0, 100, 100};
double i = 0;

void clear(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderClear(renderer);
    // SDL_RenderPresent(renderer);
}

void draw(SDL_Renderer *renderer, SDL_Rect *rect)
{
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, rect);
    SDL_RenderPresent(renderer);
}
void Renderer::Render()
{
    // clear(sdl_renderer);
    i += 0.1;
    rect.x = (int)i;

    clear(sdl_renderer_);
    draw(sdl_renderer_, &rect);
}

Renderer::~Renderer()
{
    logger::warn("Renderer destroyed");
    vkDestroyDevice(gpu_, nullptr);
    vkDestroyInstance(vulkan_instance_, nullptr);
    SDL_DestroyWindow(window_);
    SDL_DestroyRenderer(sdl_renderer_);
}