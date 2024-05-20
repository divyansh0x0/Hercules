#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <set>
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
    std::vector<VkFramebuffer> framebuffers;
};
struct QueueFamiliyIndices
{
    int GraphicsFamilyIndex;
    int PresentFamilyIndex;
    bool AreAllFamiliesAvailable(){
        return GraphicsFamilyIndex >= 0 && PresentFamilyIndex >= 0; 
    };
    //Fills given set with available queue indices
    void FillSetWithAvailableIndices(std::set<uint32_t> &set){
        if(GraphicsFamilyIndex >= 0)
            set.insert(static_cast<uint32_t>(GraphicsFamilyIndex));
        if(PresentFamilyIndex >= 0)
            set.insert(static_cast<uint32_t>(PresentFamilyIndex));
            
    }
};

struct GPUData
{
    VkPhysicalDevice physical_device;
    VkDevice device;
    std::string name;
    std::string device_type;
    QueueFamiliyIndices queue_family_indices;
    VkQueue GraphicsQueue;
    VkQueue PresentQueue;
};
struct PipelineData
{
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
    VkRenderPass render_pass;
    std::vector<VkShaderModule> vert_shader_modules;
    std::vector<VkShaderModule> frag_shader_modules;
};
class Renderer
{
private:
    VkInstance vulkan_instance_;
    GPUData gpu_data_;
    PipelineData pipeline_data_;
    VkSurfaceKHR vulkan_surface_;
    SwapChainData swapchain_data_;
    VkDebugUtilsMessengerEXT debug_messenger_;

    VkCommandPool command_pool_;
    VkCommandBuffer command_buffer_;
    VkFence in_flight_fence_;
    VkSemaphore image_available_semaphore_;
    VkSemaphore render_finished_semaphore_;
    void CreateVulkanSurfaceAndInstance(SDL_Window *window);
    void CreateGPUData();
    void CreateSwapChainData(SDL_Window *window);
    void CreatePipelineData();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffer();
    void CreateSyncObjects();
    void RecordCommandBuffer(VkCommandBuffer &command_buffer, uint32_t image_index);
    void DrawFrame();

public:
    Renderer();
    bool Initialize(SDL_Window *window);
    void Draw();
    void Destroy();
    ~Renderer();
};