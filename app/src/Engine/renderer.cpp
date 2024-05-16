#include "Engine/renderer.h"
#include "Engine/vulkanHelper.h"
#include <SDL2/SDL_vulkan.h>
#include "logger.h"
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <SDL2/SDL_syswm.h>
#include <vulkan/vulkan_win32.h>
#include <limits>
#include <algorithm>
#include <Engine/shader.h>
const bool enableValidationLayers = true;
#define ARRAY_LENGTH(arr) sizeof(arr) / sizeof(arr[0])

// bool enable_debug_logs = true;

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


///-----------------------------------------------------------------------------------------
//                                          Utility methods
//------------------------------------------------------------------------------------------
bool GPUHasAllFeatures(VkPhysicalDevice gpu)
{
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(gpu, &deviceFeatures);
    return deviceFeatures.geometryShader;
}
SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }
    return details;
}
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
                    VK_NULL_HANDLE,
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
GPUData GetOneSuitableGPU(std::vector<GPUData> gpus, VkInstance vulkan_instance, VkSurfaceKHR vulkan_surface)
{
    bool swapChainAdequate = false;
    for (GPUData gpu_info : gpus)
    {
        // Check for GPU suitablity by checking for required extensions, swap chain support and availablity of required queues
        QueueFamilyInfo gpu_queue_info = GetQueueFamilyInfo(gpu_info.physical_device, vulkan_instance, vulkan_surface);
        if (gpu_queue_info.are_all_queues_available && AreExtensionsSupported(gpu_info.physical_device))
        {
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(gpu_info.physical_device, vulkan_surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            if (swapChainAdequate)
            {
                gpu_info.queue_family_info = gpu_queue_info;
                return gpu_info;
            }
        }
    }
    logger::error("no suitable GPU found, returning first gpu");
    return gpus.at(0);
}
VkPresentModeKHR GetSuitableSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{

    // VK_PRESENT_MODE_MAILBOX_KHR: Instead of blocking the application when the queue is full,
    // the images that are already queued are simply replaced with the newer ones.
    //  This mode can be used to render frames as fast as possible while still avoiding tearing,
    // resulting in fewer latency issues than standard vertical sync.
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }
    // Only the VK_PRESENT_MODE_FIFO_KHR mode is guaranteed to be available, so its the fallback mode

    return VK_PRESENT_MODE_FIFO_KHR;
}
VkSurfaceFormatKHR GetSuitableSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    // Chooses SRGB color format
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }
    // if SRGB is not available then return the first format
    return availableFormats[0];
}

VkExtent2D GetSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, SDL_Window *window)
{
    if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
    {
        int width, height;
        SDL_Vulkan_GetDrawableSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
    return capabilities.currentExtent;
}
///-----------------------------------------------------------------------------------------
//                                          Initializer methods
//------------------------------------------------------------------------------------------
/**
 * Stores VkSwapchainKHR in gpu_info
 */

bool InitVulkanInstance(SDL_Window *window, VkInstance *vulkan_instance)
{

    // initialize vulkanInstance variable
    logger::info("INITIALIZING RENDERER");
    unsigned int extension_count;
    SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr);
    std::vector<const char *> extension_names(extension_count);

    if (SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extension_names.data()) == SDL_FALSE)
    {
        logger::error("Instance extensions could not be created: " + std::string(SDL_GetError()));
    }
    if (extension_count == 0)
    {
        logger::error("No extensions are available");
        return false;
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
    if (VkResult instance_creation_code = vkCreateInstance(&vulkan_instance_info, nullptr, vulkan_instance); instance_creation_code != VK_SUCCESS)
    {
        logger::error("Vulkan CreateInstance() failed: " + getTranslatedErrorCode(instance_creation_code));
        return false;
    }
    return true;
}
GPUData CreateGPUData(VkInstance vulkan_instance, VkSurfaceKHR vulkan_surface)
{
    // Finding a gpu with vulkan support
    std::vector<GPUData> gpus = GetSuitableGPUs(vulkan_instance);
    GPUData suitable_gpu_info = GetOneSuitableGPU(gpus, vulkan_instance, vulkan_surface); // chosing first gpu as suitable gpus
    if (gpus.empty())
    {
        logger::error("No suitable GPU found for vulkan");
    }
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
        nullptr,                                                 // ppEnabledLayerNames
        static_cast<uint32_t>(kRequiredDeviceExtensions.size()), // enabledExtensionCount
        kRequiredDeviceExtensions.data(),                        // ppEnabledExtensionNames
        &deviceFeatures,                                         // pEnabledFeatures
    };

    // creates a device and stores it in global gpu_
    VkResult device_creation_result = vkCreateDevice(suitable_gpu_info.physical_device, &device_create_info, nullptr, &suitable_gpu_info.device);
    if (device_creation_result != VK_SUCCESS)
    {
        logger::error("DEVICE CREATION FAILED ERRORCODE: " + std::to_string(device_creation_result) + " | " + getTranslatedErrorCode(device_creation_result));
    }

    return suitable_gpu_info;
}
bool CreateSwapChainViews(GPUData gpu_data)
{
    VkImageViewCreateInfo create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

    // The viewType parameter allows you to treat images as 1D textures, 2D textures, 3D textures and cube maps.
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = gpu_data.swap_chain_data.image_format;
    // The subresourceRange field describes what the image's purpose is and which part of the image should be accessed.
    //  Our images will be used as color targets without any mipmapping levels or multiple layers.
    create_info.components = VkComponentMapping{
        VK_COMPONENT_SWIZZLE_IDENTITY, //  r
        VK_COMPONENT_SWIZZLE_IDENTITY, // g
        VK_COMPONENT_SWIZZLE_IDENTITY, // b
        VK_COMPONENT_SWIZZLE_IDENTITY  // a
    };

    // The subresourceRange field describes what the image's purpose is and which part of the image should be accessed.
    //  Our images will be used as color targets without any mipmapping levels or multiple layers.
    // If you were working on a stereographic 3D application, then you would create a swap chain with multiple layers.
    // You could then create multiple image views for each image representing the views for the left and right eyes by accessing different layers.
    create_info.subresourceRange = VkImageSubresourceRange{
        VK_IMAGE_ASPECT_COLOR_BIT, // aspectMask;
        0,                         // baseMipLevel;
        1,                         // levelCount;
        0,                         // baseArrayLayer;
        1                          // layerCount;
    };
    std::vector<VkImage> sc_images = gpu_data.swap_chain_data.images;
    std::vector<VkImageView> sc_views = gpu_data.swap_chain_data.image_views;

    for (size_t i = 0; i < gpu_data.swap_chain_data.images.size(); i++)
    {
        create_info.image = sc_images[i];
        if (vkCreateImageView(gpu_data.device, &create_info, nullptr, &sc_views[i]) != VK_SUCCESS)
        {
            logger::error("Failed to create image view for image at index:" + std::to_string(i));
            return false;
        }
    }
    return true;
}
bool CreateSwapChain(GPUData gpu_data, SDL_Window *window, VkSurfaceKHR surface)
{
    SwapChainSupportDetails swap_chain_support = QuerySwapChainSupport(gpu_data.physical_device, surface);

    VkSurfaceFormatKHR surface_format = GetSuitableSwapSurfaceFormat(swap_chain_support.formats);
    VkPresentModeKHR present_mode = GetSuitableSwapPresentMode(swap_chain_support.presentModes);
    VkExtent2D extent = GetSwapExtent(swap_chain_support.capabilities, window);

    uint32_t imageCount = swap_chain_support.capabilities.minImageCount + 1;
    if (swap_chain_support.capabilities.maxImageCount > 0 && imageCount > swap_chain_support.capabilities.maxImageCount)
        imageCount = swap_chain_support.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,      // sType;
        nullptr,                                          // pNext;
        0,                                                // flags;
        surface,                                          // surface;
        imageCount,                                       // minImageCount;
        surface_format.format,                            // imageFormat;
        surface_format.colorSpace,                        // imageColorSpace;
        extent,                                           // imageExtent;
        1,                                                // imageArrayLayers;
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,              // imageUsage;
        VK_SHARING_MODE_EXCLUSIVE,                        // imageSharingMode;
        0,                                                // queueFamilyIndexCount;
        nullptr,                                          // pQueueFamilyIndices;
        swap_chain_support.capabilities.currentTransform, // preTransform;
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,                // compositeAlpha;
        present_mode,                                     // presentMode;
        VK_TRUE,                                          // clipped;
        VK_NULL_HANDLE                                    // oldSwapchain;
    };
    gpu_data.swap_chain_data = SwapChainData{};
    SwapChainData sc_data = gpu_data.swap_chain_data;
    // save extent and image format
    sc_data.extent = extent;
    sc_data.image_format = surface_format.format;
    // store swap chain
    if (vkCreateSwapchainKHR(gpu_data.device, &createInfo, nullptr, &sc_data.swap_chain) != VK_SUCCESS)
    {
        return false;
    }
    // store VkImages in SwapChainData
    vkGetSwapchainImagesKHR(gpu_data.device, sc_data.swap_chain, &imageCount, nullptr);
    sc_data.images.resize(imageCount);
    sc_data.image_views.resize(imageCount);
    vkGetSwapchainImagesKHR(gpu_data.device, sc_data.swap_chain, &imageCount, sc_data.images.data());

    return CreateSwapChainViews(gpu_data);
}

bool CreateGraphicsPipeline(){
    std::vector<char> vertShaderCode = ReadShaderFile("shaders/vert.spv");
    std::vector<char> fragShaderCode = ReadShaderFile("shaders/frag.spv");
    return false;
}

///-----------------------------------------------------------------------------------------
//                                          Public methods
//------------------------------------------------------------------------------------------

Renderer::Renderer()
{
    logger::success("Renderer object created");
}

bool Renderer::Initialize(SDL_Window *window)
{
    this->window_ = window;
    this->sdl_renderer_ = SDL_CreateRenderer(window, -1, 0);

    if (window_ == nullptr || sdl_renderer_ == nullptr)
    {
        logger::error(SDL_GetError());
        return false;
    }
    // initialize vulkan
    if (!InitVulkanInstance(window_, &vulkan_instance_))
    {
        logger::error("Vulkan initialization failed");
        return false;
    }
    logger::success("Vulkan instance inititialized successfully");

    // Create vulkan surface;
    if (SDL_Vulkan_CreateSurface(window_, vulkan_instance_, &vulkan_surface_) == SDL_FALSE)
    {
        logger::error(SDL_GetError());
    }
    logger::success("Vulkan surface inititialized successfully");

    // Create gpu data
    gpu_data_ = CreateGPUData(vulkan_instance_, vulkan_surface_);

    // Store logical device in gpu data
    VkQueue graphics_queue;
    vkGetDeviceQueue(gpu_data_.device, gpu_data_.queue_family_info.indices[0], 0, &graphics_queue);
    VkQueue present_queue;
    vkGetDeviceQueue(gpu_data_.device, gpu_data_.queue_family_info.indices[1], 0, &present_queue);

    // Swap chains

    if (!CreateSwapChain(gpu_data_, window_, vulkan_surface_))
    {
        logger::error("Failed to create swap chain!");
        return false;
    }
    // logger::success("Swapchain created successfully");

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
    i += 0.1;
    my_rect.x = static_cast<int>(i);

    clear(sdl_renderer_);
    draw(sdl_renderer_, &my_rect);
}

Renderer::~Renderer()
{
    logger::warn("Renderer destroyed");
    for (auto imageView : gpu_data_.swap_chain_data.image_views)
    {
        vkDestroyImageView(gpu_data_.device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(gpu_data_.device, gpu_data_.swap_chain_data.swap_chain, nullptr);
    vkDestroyDevice(gpu_data_.device, nullptr);
    vkDestroyInstance(vulkan_instance_, nullptr);
    SDL_DestroyWindow(window_);
    SDL_DestroyRenderer(sdl_renderer_);
}