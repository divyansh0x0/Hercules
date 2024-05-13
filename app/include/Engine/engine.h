#pragma once
#include <SDL2/SDL.h>
#include<Engine/renderer.h>
// #include <vulkan/vk_types.h>
// #include <vk_initializers.h>
#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			std::cout <<"Detected Vulkan error: " << err << std::endl; \
			abort();                                                \
		}                                                           \
	} while (0)


class Engine
{
private:
    Renderer *renderer;
    void doInput();

public:
    SDL_Window *window = NULL;
    SDL_Renderer *sdl_renderer = NULL;
    bool isRunning = false;
    Engine(int width, int height);
    void showWindow();
    void hideWindow();
    void destroy();
    void loop();
    ~Engine();
};