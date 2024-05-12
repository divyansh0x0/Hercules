// for initializing and shutdown functions
#define SDL_MAIN_HANDLED
#include <stdio.h> /* printf and fprintf */
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <stdint.h>
#include <vulkan/vulkan.h>
#include <MUI/include/window.h>

/* Sets constants */
#define WIDTH 800
#define HEIGHT 600
#define DELAY 3000
SDL_Window *window;
uint8_t isRunning = 0;
char *window_name = "example SDL2 VULKAN APP";
int KEYS[322];

int main(int argc, char **argv)
{
    window = getWindow(window_name);
    isRunning = 2;
    while (isRunning != 0)
    {
        SDL_Event windowEvent;
        while (SDL_PollEvent(&windowEvent))
        {
            if (windowEvent.type == SDL_QUIT)
            {
                isRunning = 0;
                break;
            }
        }
    }
    /* Pauses all SDL subsystems for a variable amount of milliseconds */
    SDL_Delay(DELAY);
}