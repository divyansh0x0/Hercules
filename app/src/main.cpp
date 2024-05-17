// for initializing and shutdown functions
#define SDL_MAIN_HANDLED
#include "logger.h"
#include "Hercules/engine.h"
#include "Hercules/window.h"

int main(int argc, char *argv[])
{

    hercules::Window window("Hercules", 1000, 500);
    // window.ShowWindow();
    bool quit = false;

    Engine engine(&window);
    // renderer.Initialize(window.get_sdl_window());
    // logger::info("hello");
    window.ShowWindow();
    // SDL_Event event;
    // while (!quit)
    // {
    //     while (SDL_PollEvent(&event))
    //     {
    //         switch (event.type)
    //         {
    //         case SDL_QUIT:
    //             quit = true;
    //             break;

    //         default:
    //             window.ProcessInput(event);
    //             break;
    //         }
    //     }
    // }

    // window.ShowWindow();
    engine.Loop();
    return 0;
}
