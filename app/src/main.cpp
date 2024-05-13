// for initializing and shutdown functions
#define SDL_MAIN_HANDLED
// #include <stdio.h> /* printf and fprintf */
// #include <SDL2/SDL.h>
// #include <SDL2/SDL_vulkan.h>
// #include <stdint.h>
// #include <vulkan/vulkan.h>
// #include <MUI/include/window.h>

/* Sets constants */
// #define WIDTH 800
// #define HEIGHT 600
// #define DELAY 3000
// SDL_Window *window;
// uint8_t isRunning = 0;
// char *window_name = "example SDL2 VULKAN APP";
// int KEYS[322];
#include <logger.h>
#include <Engine/engine.h>

// void handleEvent(SDL_Event event)
// {
//     switch (event.type)
//     {
//     case SDL_QUIT:
//         isRunning = false;
//         break;
//     default:
//         break;
//     }
// }

int main(int argc, char *argv[])
{
    Engine engine(1000,800);

    engine.showWindow();
    // logger::info("Hello world");
    // logger::warn("Hello world");

    // logger::error("Hello world");
    // logger::success("Hello world");


    engine.loop();
    return 0;
    // while (isRunning != 0)
    // {
    //     SDL_Event windowEvent;
    //     while (SDL_PollEvent(&windowEvent))
    //     {
    //         if (windowEvent.type == SDL_QUIT)
    //         {
    //             isRunning = 0;
    //             break;
    //         }
    //     }
    // }
}
