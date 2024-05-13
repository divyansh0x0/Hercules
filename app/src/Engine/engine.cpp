#include <Engine/engine.h>
#include <logger.h>
#include <string>
#include <chrono>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#define WINDOW_NAME "Hercules"
///-----------------------------------------------------------------------------------------
//                                          Utility methods
//------------------------------------------------------------------------------------------
void update(unsigned int dt);
SDL_Window *initAndGetSDLWindow(int width, int height)
{
    /* Initialises data */
    SDL_Window *window = NULL;
    int sdlInit = SDL_Init(SDL_INIT_VIDEO);

    /*
     * Initialises the SDL video subsystem (as well as the events subsystem).
     * Returns 0 on success or a negative error code on failure using SDL_GetError().
     */
    // SDL_Vulkan_LoadLibrary(nullotr)
    window = SDL_CreateWindow(WINDOW_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
    if (sdlInit != 0)
    {
        // fprintf(stderr, "SDL failed to initialise: %s\n", SDL_GetError());
        return NULL;
    }

    /* Checks if window has been created; if not, exits program */
    if (window == NULL)
    {
        // fprintf(stderr, "SDL window failed to initialise: %s\n", SDL_GetError());
        return NULL;
    }
    const char *errors = SDL_GetError();
    // if (errors)
    //     SDL_Log("cleaned up with errors: %s", errors);
    return window;
}
///-----------------------------------------------------------------------------------------
//                                          Public methods
//------------------------------------------------------------------------------------------

Engine::Engine(int width, int height)
{
    window = initAndGetSDLWindow(width, height);
    renderer = new Renderer(window);
}
Engine::~Engine()
{
    destroy();
}
void Engine::showWindow()
{
    isRunning = true;
    SDL_ShowWindow(window);
}
void Engine::hideWindow()
{
    SDL_HideWindow(window);
}
void Engine::destroy()
{
    isRunning = false;

    SDL_DestroyWindow(window);
}

// game loop
void Engine::loop()
{
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    unsigned int dt = 0;
    renderer->init();
    while (isRunning)
    {

        doInput();
        // SDL_UpdateWindowSurface(window);
        if (dt > 1000 / 60)
        {
            update(dt);
            renderer->render();
            auto end = high_resolution_clock::now();
            start = end;
        }
        else
        {
            dt = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
        }
    }
}

void update(unsigned int dt)
{
}

///-----------------------------------------------------------------------------------------
//                                          Private methods
//------------------------------------------------------------------------------------------

void Engine::doInput()
{
    SDL_Event window_event;

    while (SDL_PollEvent(&window_event) > 0)
    {
        // log::info("Event: " + std::to_string(e.type));
        switch (window_event.type)
        {
        case SDL_QUIT:
            logger::info(std::to_string(isRunning));
            isRunning = false;
            logger::info(std::to_string(isRunning));
            break;

        default:
            break;
        }
    }
}
// #include<log.h>
