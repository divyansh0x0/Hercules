#include "Hercules/window.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <stdexcept>
#include <logger.h>
using namespace hercules;

Window::Window(std::string window_name, int width, int height)
{
    /* Initialises data */
    // SDL_Vulkan_LoadLibrary(NULL);
    int sdlInit = SDL_Init(SDL_INIT_VIDEO);

    /*
     * Initialises the SDL video subsystem (as well as the events subsystem).
     * Returns 0 on success or a negative error code on failure using SDL_GetError().
     */
    // SDL_Vulkan_LoadLibrary(nullotr)
    if (sdlInit != 0)
    {
        throw std::runtime_error(SDL_GetError());
    }
    logger::success("SDL INITIALIZED");

    this->sdl_window_ = SDL_CreateWindow(window_name.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN| SDL_WINDOW_SHOWN);

    /* Checks if window has been created; if not, exits program */
    if (this->sdl_window_ == nullptr)
    {
        throw std::runtime_error(SDL_GetError());
    }

    this->sdl_renderer_ = SDL_CreateRenderer(sdl_window_, -1, 0);
    if (sdl_renderer_ == nullptr)
    {
        throw std::runtime_error(SDL_GetError());
    }
    logger::success("Window successfully created");
}

void Window::ProcessInput(SDL_Event event)
{
    switch (event.type)
    {
    case SDL_QUIT:
        break;

    default:
        break;
    }
}
SDL_Window *Window::get_sdl_window()
{
    return this->sdl_window_;
}
SDL_Renderer *Window::get_renderer()
{
    return this->sdl_renderer_;
}
void Window::ShowWindow()
{
    logger::info("Showing window");
    if(sdl_window_ == nullptr){
        throw std::runtime_error("Window is null" + std::string(SDL_GetError()));
    }
    SDL_ShowWindow(get_sdl_window());
    this->window_visible = true;
}
void Window::HideWindow()
{
    SDL_HideWindow(get_sdl_window());
    this->window_visible = false;
}
void Window::CloseWindow()
{
    SDL_DestroyRenderer(get_renderer());
    SDL_DestroyWindow(get_sdl_window());
    // SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}
Window::~Window()
{
    this->CloseWindow();
}