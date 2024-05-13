#pragma once
// #include<SDL2/SDL.h>
struct SDL_Window;
struct SDL_Renderer;
class Renderer{
    private:
        SDL_Renderer* sdl_renderer;
        SDL_Window* window;
    public:
        Renderer(SDL_Window* window);
        void init();
        void render();
        ~Renderer();
};