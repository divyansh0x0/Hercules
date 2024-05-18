#pragma once

#include <SDL2/SDL.h>
#include<string>

namespace hercules
{
    class Window
    {
    private:
        SDL_Window* sdl_window_;
        SDL_Renderer* sdl_renderer_;

    public:
        bool window_visible = false;
        Window(std::string window_name, int width, int height);
        ~Window();
        SDL_Window *get_sdl_window();
        SDL_Renderer *get_renderer();
        void ShowWindow();
        void HideWindow();
        void CloseWindow();
        void ProcessInput(SDL_Event event);
    };
}