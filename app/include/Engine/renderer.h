#pragma once
#include<SDL2/SDL.h>
#include<vulkan/vulkan_core.h>
// struct SDL_Window;
// struct SDL_Renderer;

class Renderer{
    private:
        struct VkInstance_T* vulkan_instance_;
        struct VkDevice_T* gpu_;
        SDL_Renderer* sdl_renderer_;
        SDL_Window* window_;
    public:
        Renderer(SDL_Window* window);
        bool Initialize();
        void Render();
        ~Renderer();
};