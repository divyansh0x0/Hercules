#include "Hercules/renderer.h"
#include "Hercules/vulkanHelper.h"
#include "Hercules/shader.h"
#include "Hercules/window.h"
#include "logger.h"

#include <iostream>

#include <SDL2/SDL_vulkan.h>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <SDL2/SDL_syswm.h>
#include <vulkan/vulkan_win32.h>
#include <limits>
#include <algorithm>

const char *kValidationLayers[] = {"VK_LAYER_KHRONOS_validation"};

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
bool AreExtensionsSupported(VkPhysicalDevice &gpu)
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
QueueFamilyInfo GetQueueFamilyInfo(VkPhysicalDevice &gpu, VkInstance &vulkan_instance, VkSurfaceKHR &surface)
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
bool GPUHasAllFeatures(VkPhysicalDevice &gpu)
{
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(gpu, &deviceFeatures);
    return deviceFeatures.geometryShader;
}
SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice &device, VkSurfaceKHR &surface)
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
void GetSuitableGPUs(VkInstance &vulkan_instance, std::vector<GPUData> &suitable_devices_info)
{

    uint32_t physical_device_count;
    vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, nullptr);
    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(vulkan_instance, &physical_device_count, physical_devices.data());
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
}
GPUData GetOneSuitableGPU(std::vector<GPUData> &gpus, VkInstance &vulkan_instance, VkSurfaceKHR &vulkan_surface)
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

VkRenderPass CreateRenderPass(GPUData &gpu_data, SwapChainData &swap_chain_data)
{
    // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment
    // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swap chain
    // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: Images to be used as destination for a memory copy operation
    VkAttachmentDescription color_attachement{
        0,                                // flags;
        swap_chain_data.image_format,     // format;
        VK_SAMPLE_COUNT_1_BIT,            // samples;
        VK_ATTACHMENT_LOAD_OP_CLEAR,      // loadOp;
        VK_ATTACHMENT_STORE_OP_STORE,     // storeOp;
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,  // stencilLoadOp;
        VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencilStoreOp;
        VK_IMAGE_LAYOUT_UNDEFINED,        // initialLayout;
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,  // finalLayout;
    };
    VkAttachmentReference color_attachement_ref{
        0,                                        // attachment
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // layout
    };

    // pInputAttachments: Attachments that are read from a shader
    // pResolveAttachments: Attachments used for multisampling color attachments
    // pDepthStencilAttachment: Attachment for depth and stencil data
    // pPreserveAttachments: Attachments that are not used by this subpass, but for which the data must be preserved
    VkSubpassDescription subpass_description{
        0,                               // flags;
        VK_PIPELINE_BIND_POINT_GRAPHICS, // pipelineBindPoint;
        0,                               // inputAttachmentCount;
        nullptr,                         // pInputAttachments;
        1,                               // colorAttachmentCount;
        &color_attachement_ref,          // pColorAttachments;
        nullptr,                         // pResolveAttachments;
        nullptr,                         // pDepthStencilAttachment;
        0,                               // preserveAttachmentCount;
        nullptr,                         // pPreserveAttachments;
    };

    VkPipelineLayout pipeline_layout;
    VkRenderPassCreateInfo render_pass_create_info{
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, // sType;
        nullptr,                                   // pNext;
        0,                                         // flags;
        1,                                         // attachmentCount;
        &color_attachement,                        // pAttachments;
        1,                                         // subpassCount;
        &subpass_description,                      // pSubpasses;
        0,                                         // dependencyCount;
        nullptr,                                   // pDependencies;
    };
    VkRenderPass render_pass;
    vkCreateRenderPass(gpu_data.device, &render_pass_create_info, nullptr, &render_pass);

    return render_pass;
}

///-----------------------------------------------------------------------------------------
//                                          Public methods
//------------------------------------------------------------------------------------------
Renderer::Renderer()
{
}
void Renderer::CreateVulkanSurfaceAndInstance(SDL_Window *window)
{
    logger::info("Initializing vulkan: ");
    unsigned int extension_count;
    SDL_Vulkan_GetInstanceExtensions(window, &extension_count, nullptr);
    std::vector<const char *> extension_names(extension_count);

    if (SDL_Vulkan_GetInstanceExtensions(window, &extension_count, extension_names.data()) == SDL_FALSE)
    {
        throw std::runtime_error("Instance extensions could not be created: " + std::string(SDL_GetError()));
    }
    if (extension_count == 0)
    {
        throw std::runtime_error("No extensions are available");
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
    if (VkResult instance_creation_code = vkCreateInstance(&vulkan_instance_info, nullptr, &this->vulkan_instance_); instance_creation_code != VK_SUCCESS)
    {
        throw std::runtime_error("Vulkan CreateInstance() failed: " + getTranslatedErrorCode(instance_creation_code));
    }

    logger::success("Vulkan instance inititialized successfully");

    // Create vulkan surface;
    if (SDL_Vulkan_CreateSurface(window, this->vulkan_instance_, &this->vulkan_surface_) == SDL_FALSE)
    {
        logger::error(SDL_GetError());
    }
    logger::success("Vulkan surface inititialized successfully");
}
void Renderer::CreateGPUData()
{
    // Finding a gpu with vulkan support
    std::vector<GPUData> gpus{};
    GetSuitableGPUs(this->vulkan_instance_, gpus);
    if (gpus.empty())
    {
        std::runtime_error("No suitable GPU found for vulkan");
    }
    this->gpu_data_ = GetOneSuitableGPU(gpus, this->vulkan_instance_, this->vulkan_surface_); // chosing first gpu as suitable gpus
    gpus.clear();

    // Create queue families
    logger::info("Creating queue families");
    const float queue_priority_level = 1.0f;
    QueueFamilyInfo queue_family_info = this->gpu_data_.queue_family_info;
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
    VkPhysicalDeviceFeatures device_features{};

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
        &device_features,                                        // pEnabledFeatures
    };

    // creates a device and stores it in this->gpu_data_
    VkResult device_creation_result = vkCreateDevice(this->gpu_data_.physical_device, &device_create_info, nullptr, &this->gpu_data_.device);
    if (device_creation_result != VK_SUCCESS)
    {
        std::runtime_error("DEVICE CREATION FAILED ERRORCODE: " + std::to_string(device_creation_result) + " | " + getTranslatedErrorCode(device_creation_result));
    }
}
void Renderer::CreateSwapChainData(SDL_Window *window)
{
    SwapChainSupportDetails swap_chain_support = QuerySwapChainSupport(this->gpu_data_.physical_device, this->vulkan_surface_);

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
        this->vulkan_surface_,                            // surface;
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
    // save extent and image format
    this->swapchain_data_.extent = extent;
    this->swapchain_data_.image_format = surface_format.format;
    // store swap chain
    if (vkCreateSwapchainKHR(this->gpu_data_.device, &createInfo, nullptr, &this->swapchain_data_.swap_chain) != VK_SUCCESS)
    {
        throw std::runtime_error("SwapchainKHR couldn't be created");
    }
    // store VkImages in SwapChainData
    vkGetSwapchainImagesKHR(this->gpu_data_.device, this->swapchain_data_.swap_chain, &imageCount, nullptr);
    this->swapchain_data_.images.resize(imageCount);
    this->swapchain_data_.image_views.resize(imageCount);
    vkGetSwapchainImagesKHR(this->gpu_data_.device, this->swapchain_data_.swap_chain, &imageCount, this->swapchain_data_.images.data());

    // Swap chain views
    VkImageViewCreateInfo create_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};

    // The viewType parameter allows you to treat images as 1D textures, 2D textures, 3D textures and cube maps.
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = this->swapchain_data_.image_format;
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
    std::vector<VkImage>* sc_images = &this->swapchain_data_.images;
    std::vector<VkImageView>* sc_views = &this->swapchain_data_.image_views;

    for (size_t i = 0; i < this->swapchain_data_.images.size(); i++)
    {
        create_info.image = (*sc_images)[i];
        if (vkCreateImageView(this->gpu_data_.device, &create_info, nullptr, &(*sc_views)[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image view for image at index:" + std::to_string(i));
        }
    }

}
void Renderer::CreatePipelineData()
{
    VkShaderModule vert_shader_module = CreateShaderModule("E:/Projects/Hercules/out/shaders/vertex_shader.spirv", this->gpu_data_.device);
    VkShaderModule frag_shader_module = CreateShaderModule("E:/Projects/Hercules/out/shaders/fragment_shader.spirv", this->gpu_data_.device);

    logger::success("Shader modules created successfully");
    // The pName and pSpecializationInfo; specify the shader module containing the code, and the function to invoke, known as the entrypoint.
    // That means that it's possible to combine multiple fragment shaders into a single shader module and use
    // different entry points to differentiate between their behaviors. In this case we'll stick to the standard main, however.

    // flags: is a bitmask of VkPipelineShaderStageCreateFlagBits specifying how the pipeline shader stage will be generated.
    // stage: is a VkShaderStageFlagBits value specifying a single pipeline stage.
    // module: is optionally a VkShaderModule object containing the shader code for this stage.
    // pName: is a pointer to a null-terminated UTF-8 string specifying the entry point name of the shader for this stage.
    // pSpecializationInfo: is a pointer to a VkSpecializationInfo structure, as described in Specialization Constants, or NULL.
    VkPipelineShaderStageCreateInfo vert_shader_stage_info{
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType;
        nullptr,                                             // pNext;
        0,                                                   // flags;
        VK_SHADER_STAGE_VERTEX_BIT,                          // stage;
        vert_shader_module,                                  // module;
        "main",                                              // pName;
        nullptr                                              // pSpecializationInfo;
    };
    VkPipelineShaderStageCreateInfo frag_shader_stage_info{
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType;
        nullptr,                                             // pNext;
        0,                                                   // flags;
        VK_SHADER_STAGE_FRAGMENT_BIT,                        // stage;
        frag_shader_module,                                  // module;
        "main",                                              // pName;
        nullptr                                              // pSpecializationInfo;
    };
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, // sType
        nullptr,                                              // pNext
        0,                                                    // flags
        static_cast<uint32_t>(kDynamicStates.size()),         // dynamicStateCount
        kDynamicStates.data()                                 // pDynamicStates
    };
    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, // sType
        nullptr,                                                   // pNext
        0,                                                         // flags
        0,                                                         // vertexBindingDescriptionCount
        nullptr,                                                   // pVertexBindingDescriptions
        0,                                                         // vertexAttributeDescriptionCount
        nullptr                                                    // pVertexAttributeDescriptions

    };
    //    VK_PRIMITIVE_TOPOLOGY_POINT_LIST: points from vertices
    // VK_PRIMITIVE_TOPOLOGY_LINE_LIST: line from every 2 vertices without reuse
    // VK_PRIMITIVE_TOPOLOGY_LINE_STRIP: the end vertex of every line is used as start vertex for the next line
    // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST: triangle from every 3 vertices without reuse
    // VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP: the second and third vertex of every triangle are used as first two vertices of the next triangle

    // If the primitiveRestartEnable member is set to VK_TRUE, then it's possible to break up lines and triangles in the
    // VK_PRIMITIVE_TOPOLOGY_LINE_STRIP topology modes by using a special index of 0xFFFF or 0xFFFFFFFF.
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, // sType
        nullptr,                                                     // pNext
        0,                                                           // flags
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                         // topology
        VK_FALSE,                                                    // primitiveRestartEnable
    };
    input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;
    VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};
    // A viewport basically describes the region of the framebuffer that the output will be rendered to
    VkViewport viewport{
        0.0f,                                       // x
        0.0f,                                       // y
        (float)this->swapchain_data_.extent.width,  // width
        (float)this->swapchain_data_.extent.height, // height
        0.0f,                                       // minDepth
        1.0f,                                       // maxDepth
    };
    VkRect2D scissor_rect{
        {0, 0},                      // offset VkOffset2D
        this->swapchain_data_.extent // extent VkExtent2D
    };
    // The actual viewport(s) and scissor rectangle(s) will then later be set up at drawing time.
    VkPipelineViewportStateCreateInfo viewport_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, // sType
        nullptr,                                               // pNext
        0,                                                     // flags
        1,                                                     // viewportCount
        nullptr,                                               // pViewports
        1,                                                     // scissorCount
        nullptr,                                               // pScissors
    };

    // The polygonMode determines how fragments are generated for geometry. The following modes are available:
    //  VK_POLYGON_MODE_FILL: fill the area of the polygon with fragments
    //  VK_POLYGON_MODE_LINE: polygon edges are drawn as lines
    //  VK_POLYGON_MODE_POINT: polygon vertices are drawn as points
    //  Using any mode other than fill requires enabling a GPU feature.
    VkPipelineRasterizationStateCreateInfo rasterizer_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, // sType;
        nullptr,                                                    // pNext;
        0,                                                          // flags;
        VK_FALSE,                                                   // depthClampEnable;
        VK_FALSE,                                                   // rasterizerDiscardEnable;
        VK_POLYGON_MODE_FILL,                                       // polygonMode;
        VK_CULL_MODE_BACK_BIT,                                      // cullMode;
        VK_FRONT_FACE_CLOCKWISE,                                    // frontFace;
        VK_FALSE,                                                   // depthBiasEnable;
        0.0f,                                                       // depthBiasConstantFactor;
        0.0f,                                                       // depthBiasClamp;
        0.0f,                                                       // depthBiasSlopeFactor;
        1.0f,                                                       // lineWidth;
    };
    // While viewports define the transformation from the image to the framebuffer,
    //  scissor rectangles define in which regions pixels will actually be stored.
    // Any pixels outside the scissor rectangles will be discarded by the rasterizer

    VkPipelineMultisampleStateCreateInfo multisampling_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, // sType;
        nullptr,                                                  // pNext;
        0,                                                        // flags;
        VK_SAMPLE_COUNT_1_BIT,                                    // rasterizationSamples;
        VK_FALSE,                                                 // sampleShadingEnable;
        1.0f,                                                     // minSampleShading;
        nullptr,                                                  // pSampleMask;
        VK_FALSE,                                                 // alphaToCoverageEnable;
        VK_FALSE,                                                 // alphaToOneEnable;
    };
    VkPipelineColorBlendAttachmentState colorblend_attachement{
        VK_FALSE,                                                                                                 // blendEnable;
        VK_BLEND_FACTOR_SRC_ALPHA,                                                                                // srcColorBlendFactor;
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,                                                                      // dstColorBlendFactor;
        VK_BLEND_OP_ADD,                                                                                          // colorBlendOp;
        VK_BLEND_FACTOR_ONE,                                                                                      // srcAlphaBlendFactor;
        VK_BLEND_FACTOR_ZERO,                                                                                     // dstAlphaBlendFactor;
        VK_BLEND_OP_ADD,                                                                                          // alphaBlendOp;
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT // colorWriteMask;
    };
    VkPipelineColorBlendStateCreateInfo color_blending_state_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, // sType;
        nullptr,                                                  // pNext;
        0,                                                        // flags;
        VK_FALSE,                                                 // logicOpEnable;
        VK_LOGIC_OP_COPY,                                         // logicOp;
        1,                                                        // attachmentCount;
        &colorblend_attachement,                                  // pAttachments;
        {0.0f, 0.0f, 0.0f, 0.0f},                                 // blendConstants[4];
    };

    VkPipelineLayoutCreateInfo pipeline_layout_create_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, // sType;
        nullptr,                                       // pNext;
        0,                                             // flags;
        0,                                             // setLayoutCount;
        nullptr,                                       // pSetLayouts;
        0,                                             // pushConstantRangeCount;
        nullptr,                                       // pPushConstantRanges;
    };

    VkPipelineLayout pipeline_layout;
    if (vkCreatePipelineLayout(this->gpu_data_.device, &pipeline_layout_create_info, nullptr, &pipeline_layout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
    logger::success("Pipeline Layout created successfully");
    VkRenderPass render_pass = CreateRenderPass(this->gpu_data_, this->swapchain_data_);
    logger::success("Render pass created");
    VkGraphicsPipelineCreateInfo pipeline_create_info{
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, // sType;
        nullptr,                                         // pNext;
        0,                                               // flags;
        2,                                               // stageCount;
        shader_stages,                                   // pStages;
        &vertex_input_state_create_info,                 // pVertexInputState;
        &input_assembly_state_create_info,               // pInputAssemblyState;
        nullptr,                                         // pTessellationState;
        &viewport_state_create_info,                     // pViewportState;
        &rasterizer_state_create_info,                   // pRasterizationState;
        &multisampling_state_create_info,                // pMultisampleState;
        nullptr,                                         // pDepthStencilState;
        &color_blending_state_create_info,               // pColorBlendState;
        &dynamic_state_create_info,                      // pDynamicState;
        pipeline_layout,                                 // layout;
        render_pass,                                     // renderPass;
        0,                                               // subpass; it is the index of the subpass in the render pass where this pipeline will be used.
        VK_NULL_HANDLE,                                  // basePipelineHandle;
        -1,                                              // basePipelineIndex;
    };
    VkPipeline graphics_pipeline;
    if (vkCreateGraphicsPipelines(this->gpu_data_.device, VK_NULL_HANDLE, 1, &pipeline_create_info, nullptr, &graphics_pipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    this->pipeline_data_.pipeline = graphics_pipeline;
    this->pipeline_data_.pipeline_layout = pipeline_layout;
    this->pipeline_data_.render_pass = render_pass;
    this->pipeline_data_.vert_shader_modules = {vert_shader_module};
    this->pipeline_data_.frag_shader_modules = {frag_shader_module};
}
void Renderer::CreateFramebuffers(){
    
    std::vector<VkFramebuffer> *framebuffers = &this->swapchain_data_.framebuffers;

    this->swapchain_data_.framebuffers.resize(this->swapchain_data_.image_views.size());

    for (size_t i = 0; i < this->swapchain_data_.image_views.size(); i++)
    {
        VkImageView attachments[] = {this->swapchain_data_.image_views[i]};

        VkFramebufferCreateInfo framebuffer_info{};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = pipeline_data_.render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = this->swapchain_data_.extent.width;
        framebuffer_info.height = this->swapchain_data_.extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(this->gpu_data_.device, &framebuffer_info, nullptr, &(*framebuffers)[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}
bool Renderer::Initialize(SDL_Window *window)
{

    CreateVulkanSurfaceAndInstance(window);
    CreateGPUData();
    CreateSwapChainData(window);
    logger::success("Swap chain created successfully");
    CreatePipelineData();
    logger::success("Pipeline data created successfully");
    CreateFramebuffers();
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
void Renderer::Destroy()
{
    logger::warn("Cleaning up vulkan object instances");

    vkDestroyPipelineLayout(gpu_data_.device, pipeline_data_.pipeline_layout, nullptr);
    logger::info("Pipeline layout destroyed");
    vkDestroyPipeline(gpu_data_.device, pipeline_data_.pipeline, nullptr);
    logger::info("Graphics Pipeline destroyed");
    vkDestroyRenderPass(gpu_data_.device, pipeline_data_.render_pass, nullptr);
    logger::info("Render pass destroyed");

    for (auto imageView : this->swapchain_data_.image_views)
        vkDestroyImageView(gpu_data_.device, imageView, nullptr);
    logger::info("Image views destroyed");
    for (VkShaderModule frag_shader : pipeline_data_.frag_shader_modules)
        vkDestroyShaderModule(gpu_data_.device, frag_shader, nullptr);
    logger::info("Image views destroyed");

    for (VkShaderModule vert_shader : pipeline_data_.vert_shader_modules)
        vkDestroyShaderModule(gpu_data_.device, vert_shader, nullptr);
    logger::info("Image views destroyed");

    vkDestroySwapchainKHR(gpu_data_.device, this->swapchain_data_.swap_chain, nullptr);
    logger::info("SwapchainKHR destroyed");

    vkDestroyDevice(gpu_data_.device, nullptr);
    logger::info("Vulkan device destroyed");
    vkDestroyInstance(vulkan_instance_, nullptr);
    logger::info("Vulkan instance destroyed");

    logger::success("Vulkan object instances cleaned up");
}
void Renderer::Render()
{
}

Renderer::~Renderer()
{
    this->Destroy();
}