#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>
#include <SDL2/SDL.h>
// struct SDL_Window;
// struct SDL_Renderer;
// The older graphics APIs provided default state for most of the stages of the graphics pipeline.
//  In Vulkan you have to be explicit about most pipeline states as it'll be baked into an immutable pipeline state object.
const std::vector<const char *> kRequiredDeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const std::vector<VkDynamicState> kDynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR};

const VkApplicationInfo kAapplicationInfo = {
    VK_STRUCTURE_TYPE_APPLICATION_INFO,
    nullptr,
    "Physics Engine",
    VK_MAKE_VERSION(1, 0, 0),
    "Hercules",
    VK_MAKE_VERSION(1, 0, 0),
    VK_API_VERSION_1_0

};
struct SwapChainData
{
    VkSwapchainKHR swap_chain;
    VkExtent2D extent;
    VkFormat image_format;
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
};
struct QueueFamilyInfo
{
    bool are_all_queues_available;
    //{GraphicsFamilyIndex, PresentFamilyIndex}
    int indices[2] = {-1, -1};
    uint32_t queue_count;
};

struct GPUData
{
    VkPhysicalDevice physical_device;
    VkDevice device;
    std::string name;
    std::string device_type;
    QueueFamilyInfo queue_family_info;
    SwapChainData swap_chain_data;
};
struct PipelineData
{
    VkPipelineLayout pipeline_layout;
};
class Renderer
{
private:
    VkInstance vulkan_instance_;
    GPUData gpu_data_;
    PipelineData pipeline_data_;
    VkSurfaceKHR vulkan_surface_;

public:
    Renderer();
    bool Initialize(SDL_Window *window);
    void Render();
    ~Renderer();
};