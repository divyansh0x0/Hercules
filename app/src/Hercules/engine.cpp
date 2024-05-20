#include "Hercules/engine.h"
#include "Hercules/renderer.h"
#include "Hercules/window.h"

#include "logger.h"
#include <string>
#include <chrono>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <stdexcept>

#define WINDOW_NAME "Hercules"
///-----------------------------------------------------------------------------------------
//                                          Utility methods
//------------------------------------------------------------------------------------------
void update(unsigned int dt)
{
}

///-----------------------------------------------------------------------------------------
//                                          Public methods
//------------------------------------------------------------------------------------------

Engine::Engine(hercules::Window &window):window_(window)
{
    this->is_engine_running = true;
    logger::success("Engine created");
}
Engine::~Engine()
{
    DestroyEngine();
}

void Engine::DestroyEngine()
{
    is_engine_running = false;
}

// Gives the control to game engine and shows the window if it was not visible;
void Engine::Loop()
{
    logger::info("starting game loop");

    auto start = std::chrono::high_resolution_clock::now();
    unsigned int dt = 0;

    bool is_renderer_initialized = renderer_.Initialize(window_.get_sdl_window());

    // if (!is_renderer_initialized)
    //     throw std::runtime_error("Vulkan renderer couldn't be initialized");
    SDL_Event event;
    while (is_engine_running == true)
    {


        while (SDL_PollEvent(&event))
        {
            logger::info("Proccessing input : " + std::to_string(event.type));
            switch (event.type)
            {
            case SDL_QUIT:
                is_engine_running = false;
                break;

            default:
                window_.ProcessInput(event);
                break;
            }
        }

        // window_.ProcessInput();
        if (dt > 1000 / 60)
        {
            update(dt);
            renderer_.Draw();
            auto end = std::chrono::high_resolution_clock::now();
            start = end;
        }
        else
        {
            dt = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
        }
    }
}

///-----------------------------------------------------------------------------------------
//                                          Private methods
//------------------------------------------------------------------------------------------

// #include<log.h>
