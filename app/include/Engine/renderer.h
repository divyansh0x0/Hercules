#pragma once
#include<SDL2/SDL.h>
#include<vulkan/vulkan_core.h>
// struct SDL_Window;
// struct SDL_Renderer;

class Renderer{
    private:
        VkInstance vulkan_instance_;
        VkDevice gpu_;
        SDL_Renderer* sdl_renderer_;
        SDL_Window* window_;
        VkSurfaceKHR vulkan_surface_;
    public:
        Renderer();
        bool Initialize();
        void Render();
        void set_window(SDL_Window* window);
        ~Renderer();
};