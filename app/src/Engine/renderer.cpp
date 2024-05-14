#include "Engine/renderer.h"
#include "Engine/vulkanHelper.h"
#include <SDL2/SDL_vulkan.h>
#include "logger.h"
#include <string>
#include<vector>

// #define

// #ifdef _WIN32
// // #pragma comment(linker, "/subsystem:windows")
// #define VK_USE_PLATFORM_WIN32_KHR
// #define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
// #endif

const VkApplicationInfo kAapplicationInfo = {
    VK_STRUCTURE_TYPE_APPLICATION_INFO,
    nullptr,
    "Physics Engine",
    VK_MAKE_VERSION(1, 0, 0),
    "Hercules",
    VK_MAKE_VERSION(1, 0, 0),
    VK_API_VERSION_1_0

};
bool GPUHasAllFeatures(VkPhysicalDevice gpu){
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(gpu,&deviceFeatures);
    return deviceFeatures.geometryShader;
}
///-----------------------------------------------------------------------------------------
//                                          Utility methods
//------------------------------------------------------------------------------------------
VkPhysicalDevice* GetSuitableGPUs(VkInstance vulkan_instance){
    uint32_t physicalDeviceCount;
    vkEnumeratePhysicalDevices(vulkan_instance, &physicalDeviceCount, nullptr);
    VkPhysicalDevice physicalDevices[physicalDeviceCount];
    vkEnumeratePhysicalDevices(vulkan_instance, &physicalDeviceCount, physicalDevices);

    std::vector<VkPhysicalDevice> suitableDevices;
    if (physicalDeviceCount == 0)
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
        for (VkPhysicalDevice device : physicalDevices)
        {
            vkGetPhysicalDeviceProperties(device, &device_properties);

            logger::info("\t DEVICE NAME:" + std::string(device_properties.deviceName) +
                         " | DEVICE TYPE:" + device_type_names[device_properties.deviceType] +
                         "| API VERSION:" + std::to_string(device_properties.apiVersion));
            if((device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) && GPUHasAllFeatures(device))
                suitableDevices.push_back(device);
        }
    }
    return suitableDevices.data();
}
    


///-----------------------------------------------------------------------------------------
//                                          Public methods
//------------------------------------------------------------------------------------------

Renderer::Renderer(SDL_Window *window)
{
    this->window_ = window;
    this->sdl_renderer_ = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    logger::error(SDL_GetError());
}

bool Renderer::Initialize()
{
    logger::info("INITIALIZING RENDERER");
    unsigned int extensionCount;
    SDL_Vulkan_GetInstanceExtensions(window_, &extensionCount, nullptr);
    const char *extensionNames[extensionCount];
    SDL_Vulkan_GetInstanceExtensions(window_, &extensionCount, extensionNames);

    // initialize vulkanInstance variable
    if (extensionCount == 0)
    {
        logger::error("EXTENSION COUNT IS ZERO");
    }
    else
    {
        logger::info("Available vulkan extensions:");

        for (const char *extension : extensionNames)
        {
            logger::info("\t" + std::string(extension));
        }
    }

    VkInstanceCreateInfo vulkanInstanceInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, // sType
        nullptr,                                // pNext
        0,                                      // flags
        &kAapplicationInfo,                     // pApplicationInfo
        0,                                      // enabledLayerCount
        nullptr,                                // ppEnabledLayerNames
        extensionCount,                         // enabledExtensionCount
        extensionNames                          // ppEnabledExtensionNames
    };
    VkResult instance_creation_code = vkCreateInstance(&vulkanInstanceInfo, nullptr, &vulkan_instance_);
    if (instance_creation_code != VK_SUCCESS)
    {
        logger::error("VULKAN INSTANCE CREATION FAILED! : " + getTranslatedErrorCode(instance_creation_code));
        return false;
    }

    // Finding a gpu with vulkan support
    VkPhysicalDevice* gpus = GetSuitableGPUs(vulkan_instance_);
    if(gpus == nullptr)
        return false;


    VkPhysicalDevice gpu = gpus[0];
    uint32_t queuefamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queuefamilyCount, nullptr);
    VkQueueFamilyProperties queueFamilies[queuefamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queuefamilyCount, queueFamilies);

    VkSurfaceKHR surface;
    SDL_Vulkan_CreateSurface(window_, vulkan_instance_, &surface);

    uint32_t graphicsQueueIndex = UINT32_MAX;
    uint32_t presentQueueIndex = UINT32_MAX;
    VkBool32 support;
    uint32_t i = 0;
    for (VkQueueFamilyProperties queueFamily : queueFamilies)
    {
        if (graphicsQueueIndex == UINT32_MAX && queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            graphicsQueueIndex = i;
        if (presentQueueIndex == UINT32_MAX)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &support);
            if (support)
                presentQueueIndex = i;
        }
        ++i;
    }

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
        nullptr,                                    // pNext
        0,                                          // flags
        graphicsQueueIndex,                         // graphicsQueueIndex
        1,                                          // queueCount
        &queuePriority,                             // pQueuePriorities
    };
    VkPhysicalDeviceFeatures deviceFeatures = {};
    const char *deviceExtensionNames[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, // sType
        nullptr,                              // pNext
        0,                                    // flags
        1,                                    // queueCreateInfoCount
        &queueInfo,                           // pQueueCreateInfos
        0,                                    // enabledLayerCount
        nullptr,                              // ppEnabledLayerNames
        1,                                    // enabledExtensionCount
        deviceExtensionNames,                 // ppEnabledExtensionNames
        &deviceFeatures,                      // pEnabledFeatures
    };

    vkCreateDevice(gpu, &createInfo, nullptr, &gpu_);

    VkQueue graphicsQueue;
    vkGetDeviceQueue(gpu_, graphicsQueueIndex, 0, &graphicsQueue);

    VkQueue presentQueue;
    vkGetDeviceQueue(gpu_, presentQueueIndex, 0, &presentQueue);

    logger::success("RENDERER WAS INITIALIZED SUCCESSFULLY");

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
    // logger::info(SDL_GetError());
}

Renderer::~Renderer()
{
    vkDestroyDevice(gpu_, nullptr);
    vkDestroyInstance(vulkan_instance_, nullptr);
    SDL_DestroyWindow(window_);
    SDL_DestroyRenderer(sdl_renderer_);
}