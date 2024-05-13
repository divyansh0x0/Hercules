#pragma once
#include<SDL2/SDL.h>
class Renderer{
    public:
        Renderer(SDL_Window* window);
        void update();
        void render();
};