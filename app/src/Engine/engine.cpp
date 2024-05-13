#include <Engine/engine.h>
#include <log.h>
#include <string>
#include <chrono>
///-----------------------------------------------------------------------------------------
//                                          Utility methods
//------------------------------------------------------------------------------------------
void update(SDL_Renderer *renderer, SDL_Window *window);
SDL_Window *initAndGetSDLWindow(char *window_name, int width, int height)
{
    /* Initialises data */
    SDL_Window *window = NULL;
    int sdlInit = SDL_Init(SDL_INIT_EVERYTHING);

    /*
     * Initialises the SDL video subsystem (as well as the events subsystem).
     * Returns 0 on success or a negative error code on failure using SDL_GetError().
     */
    // SDL_Vulkan_LoadLibrary(nullotr)
    window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
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
    if (errors)
        SDL_Log("cleaned up with errors: %s", errors);
    return window;
}
///-----------------------------------------------------------------------------------------
//                                          Public methods
//------------------------------------------------------------------------------------------

Engine::Engine(int width, int height)
{
    window = initAndGetSDLWindow("Hercules", width, height);
    sdl_renderer = SDL_GetRenderer(window);
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
    long long dt = 0;
    while (isRunning)
    {

        doInput();
        // SDL_UpdateWindowSurface(window);
        if (dt > 1000 / 60)
        {

            update(sdl_renderer, window);
            auto end = high_resolution_clock::now();
            start = end;
        }
        else
        {
            dt = duration_cast<milliseconds>(high_resolution_clock::now() - start).count();
        }
        log::info("dt:" + std::to_string(dt));
    }
}

void update(SDL_Renderer *renderer, SDL_Window *window)
{
    SDL_SetRenderDrawColor(renderer, 96, 128, 255, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    SDL_UpdateWindowSurface(window);
    SDL_Surface *screen = SDL_GetWindowSurface(window);

    // Draw Square
    SDL_Rect rect = { 0, 0, 70, 70 };
    SDL_FillRect(screen, &rect, SDL_MapRGB(screen -> format, 0xF0, 0xF0, 0xF0));

    // SDL_Delay(10000);
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
            log::info(std::to_string(isRunning));
            isRunning = false;
            log::info(std::to_string(isRunning));
            break;

        default:
            break;
        }
    }
}
// #include<log.h>
