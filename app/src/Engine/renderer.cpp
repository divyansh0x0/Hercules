#include <Engine/renderer.h>
#include <SDL2/SDL.h>
#include <logger.h>
Renderer::Renderer(SDL_Window *window)
{
    this->window = window;
    this->sdl_renderer = SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED);
    logger::error(SDL_GetError());
    
}

void Renderer::init()
{
}
SDL_Rect rect = {0,0,100,100};
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
void Renderer::render()
{
    // clear(sdl_renderer);
    i += 0.1;
    rect.x = (int)i;

    clear(sdl_renderer);
    draw(sdl_renderer, &rect);
    // logger::info(SDL_GetError());
}

Renderer::~Renderer()
{
    SDL_DestroyRenderer(sdl_renderer);
}