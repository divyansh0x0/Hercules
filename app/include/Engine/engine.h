#pragma once
// #include <SDL2/SDL.h>
#include "Engine/renderer.h"
// #include <vulkan/vk_types.h>
// #include <vk_initializers.h>
struct SDL_Window;

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
    Renderer* renderer_;
    void doInput();

public:
    SDL_Window *window_;
    bool is_engine_running = false;
    Engine(int width, int height);
    void ShowWindow();
    void HideWindow();
    void DestroyEngine();
    void Loop();
    ~Engine();
};