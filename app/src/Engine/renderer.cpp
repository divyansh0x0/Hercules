#include "Engine/renderer.h"
#include "Engine/vulkanHelper.h"
#include <SDL2/SDL_vulkan.h>
#include "logger.h"
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <SDL2/SDL_syswm.h>
#include <vulkan/vulkan_win32.h>
#define VK_USE_PLATFORM_WIN32_KHR
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

bool enable_debug_logs = true;
const VkApplicationInfo kAapplicationInfo = {
    VK_STRUCTURE_TYPE_APPLICATION_INFO,
    nullptr,
    "Physics Engine",
    VK_MAKE_VERSION(1, 0, 0),
    "Hercules",
    VK_MAKE_VERSION(1, 0, 0),
    VK_API_VERSION_1_0

};
const std::vector<const char *> kRequiredDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};
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
    //{GraphicsFamilyIndex, PresentFamilyIndex}
    int indices[2] = {-1, -1};
    uint32_t queue_count;
};
struct GPUData
{
    VkPhysicalDevice gpu;
    std::string name;
    std::string device_type;
    QueueFamilyInfo queue_family_info;
};
///-----------------------------------------------------------------------------------------
//                                          Utility methods
//------------------------------------------------------------------------------------------

// bool initVulkanInstance(SDL_Window *window, VkInstance vulkan_instance)
// {
// }
bool checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : kValidationLayers)
    {
        bool layerFound = false;

        logger::info("DEBUG LAYERS FOUND:");
        for (VkLayerProperties layerProperties : availableLayers)
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
std::vector<GPUData> GetSuitableGPUs(VkInstance vulkan_instance)
{

    uint32_t physical_device_count;
    vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, nullptr);
    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, physical_devices.data());

    std::vector<GPUData> suitable_devices_info;
    if (physical_device_count == 0)
    {
        logger::error("Failed to find GPUs with Vulkan support!");
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

            std::string device_type = device_type_names[device_properties.deviceType];
            logger::info("\t DEVICE NAME:" + std::string(device_properties.deviceName) +
                         " | DEVICE TYPE:" + device_type +
                         "| API VERSION:" + std::to_string(device_properties.apiVersion));
            if ((device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) && GPUHasAllFeatures(device))
            {
                GPUData gpu_info = {
                    device,
                    std::string(device_properties.deviceName),
                    device_type};
                suitable_devices_info.push_back(gpu_info);
            }
        }
    }
    return suitable_devices_info;
}

QueueFamilyInfo GetQueueFamilyInfo(VkPhysicalDevice gpu, VkInstance vulkan_instance, VkSurfaceKHR surface)
{
    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_family_count, queue_families.data());

    // VkSurfaceKHR surface;
    QueueFamilyInfo queue_family_info{};
    int i = 0;
    for (const VkQueueFamilyProperties queueFamily : queue_families)
    {
        uint32_t is_graphics_family_available = queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT;
        if (is_graphics_family_available)
        {
            queue_family_info.queue_count++;
            queue_family_info.indices[0] = i;
        }
        VkBool32 is_present_available = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &is_present_available);
        if (is_present_available)
            queue_family_info.indices[1] = i;
        queue_family_info.are_all_queues_available = is_graphics_family_available && is_present_available;
        if (queue_family_info.are_all_queues_available)
            break;
        i++;
    }
    return queue_family_info;
}
bool AreExtensionsSupported(VkPhysicalDevice gpu)
{
    // VkPhysicalDevice_T sl{};
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(gpu, nullptr, &extensionCount, availableExtensions.data());

    logger::info("CHECKING EXTENSION SUPPORT");
    uint32_t extensions_found = 0;
    for (const auto &required_extension_name : kRequiredDeviceExtensions)
    {
        for (VkExtensionProperties extension : availableExtensions)
        {
            if (strcmp(required_extension_name, extension.extensionName) == 0)
            {
                extensions_found++;
                break;
            }
        }
    }

    return extensions_found == kRequiredDeviceExtensions.size();
}
GPUData ChooseOneSuitableGPU(std::vector<GPUData> gpus, VkInstance vulkan_instance, VkSurfaceKHR vulkan_surface)
{
    for (GPUData gpu_info : gpus)
    {
        QueueFamilyInfo gpu_queue_info = GetQueueFamilyInfo(gpu_info.gpu, vulkan_instance, vulkan_surface);
        if (gpu_queue_info.are_all_queues_available && AreExtensionsSupported(gpu_info.gpu))
        {
            gpu_info.queue_family_info = gpu_queue_info;
            return gpu_info;
        }
    }
    logger::error("no suitable GPU found, returning first gpu");
    return gpus.at(0);
}
///-----------------------------------------------------------------------------------------
//                                          Public methods
//------------------------------------------------------------------------------------------

Renderer::Renderer()
{
    logger::success("Renderer object created");
}
void Renderer::set_window(SDL_Window *window)
{
    this->window_ = window;
    this->sdl_renderer_ = SDL_CreateRenderer(window, -1, 0);
    if (sdl_renderer_ == nullptr)
        logger::error(SDL_GetError());
}
bool Renderer::Initialize()
{

    if (window_ == nullptr)
    {
        logger::error("Call set_window() before initialization of renderer");
        return false;
    }

    // initialize vulkanInstance variable
    logger::info("INITIALIZING RENDERER");
    unsigned int extension_count;
    SDL_Vulkan_GetInstanceExtensions(window_, &extension_count, nullptr);
    std::vector<const char *> extension_names(extension_count);

    SDL_bool b1 = SDL_Vulkan_GetInstanceExtensions(window_, &extension_count, extension_names.data());
    if (b1)
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
    VkInstanceCreateInfo vulkan_instance_info = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, // sType
        nullptr,                                // pNext
        0,                                      // flags
        &kAapplicationInfo,                     // pApplicationInfo
        0,                                      // enabledLayerCount
        nullptr,                                // ppEnabledLayerNames
        extension_count,                        // enabledExtensionCount
        extension_names.data()                  // ppEnabledExtensionNames
    };
    VkResult instance_creation_code = vkCreateInstance(&vulkan_instance_info, nullptr, &vulkan_instance_);
    if (instance_creation_code != VK_SUCCESS)
    {
        logger::error("VULKAN INSTANCE CREATION FAILED! : " + getTranslatedErrorCode(instance_creation_code));
        return false;
    }
    logger::error(SDL_GetError());
    logger::success("Vulkan instance inititialized successfully");
    // VkSurfaceKHR vulkanSurface;
    if (SDL_Vulkan_CreateSurface(window_, vulkan_instance_, &vulkan_surface_) == SDL_FALSE)
    {
        logger::error(SDL_GetError());
    }
    logger::success("Vulkan surface inititialized successfully");

    // Finding a gpu with vulkan support
    std::vector<GPUData> gpus = GetSuitableGPUs(vulkan_instance_);
    if (gpus.empty())
    {
        logger::error("No suitable GPU found for vulkan");
        return false;
    }
    GPUData suitable_gpu_info = ChooseOneSuitableGPU(gpus, vulkan_instance_, vulkan_surface_); // chosing first gpu as suitable gpus
    logger::info("Creating queue families");
    const float queue_priority_level = 1.0f;
    QueueFamilyInfo queue_family_info = suitable_gpu_info.queue_family_info;
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    for (uint32_t i = 0; i < queue_family_info.queue_count; i++)
    {
        uint32_t queue_family_index = static_cast<uint32_t>(queue_family_info.indices[i]);
        if (queue_family_index < 0)
        { //-ve value means queue was not found
            logger::warn("Queue family of index " + std::to_string(queue_family_index) + " in QueueFamilyInfo struct was not found!");
            continue;
        }

        VkDeviceQueueCreateInfo queue_create_info = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType
            nullptr,                                    // pNext
            0,                                          // flags
            queue_family_index,                         // queueFamilyIndex
            1,                                          // queueCount
            &queue_priority_level                       // pQueuePriorities
        };
        queue_create_infos.push_back(queue_create_info);
    }
    logger::success("Queue families created!");
    logger::info("Creating VkDevice");
    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo device_create_info{
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,                    // sType
        nullptr,                                                 // pNext
        0,                                                       // flags
        static_cast<uint32_t>(queue_create_infos.size()),        // queueCreateInfoCount
        queue_create_infos.data(),                               // pQueueCreateInfos
        0,                                                       // enabledLayerCount
        nullptr,                        // ppEnabledLayerNames
        static_cast<uint32_t>(kRequiredDeviceExtensions.size()), // enabledExtensionCount
        kRequiredDeviceExtensions.data(),                                                 // ppEnabledExtensionNames
        &deviceFeatures,                                         // pEnabledFeatures
    };

    // creates a device and stores it in global gpu_
    VkResult device_creation_result = vkCreateDevice(suitable_gpu_info.gpu, &device_create_info, nullptr, &gpu_);
    if (device_creation_result != VK_SUCCESS)
    {
        logger::error("DEVICE CREATION FAILED ERRORCODE: " + std::to_string(device_creation_result) + " | " + getTranslatedErrorCode(device_creation_result));
        return false;
    }
    logger::success("DEVICE CREATED SUCCESSFULLY");

    VkQueue graphics_queue;
    vkGetDeviceQueue(gpu_, queue_family_info.indices[0], 0, &graphics_queue);
    VkQueue present_queue;
    vkGetDeviceQueue(gpu_, queue_family_info.indices[1], 0, &present_queue);
    return true;
}
SDL_Rect my_rect = {0, 0, 100, 100};
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
    my_rect.x = static_cast<int>(i);

    clear(sdl_renderer_);
    draw(sdl_renderer_, &my_rect);
}

Renderer::~Renderer()
{
    logger::warn("Renderer destroyed");
    vkDestroyDevice(gpu_, nullptr);
    vkDestroyInstance(vulkan_instance_, nullptr);
    SDL_DestroyWindow(window_);
    SDL_DestroyRenderer(sdl_renderer_);
}