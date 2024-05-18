#pragma once
#include "Hercules/renderer.h"
#include "Hercules/window.h"
#include <string>
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
    bool is_engine_running = false;
    hercules::Window window_;
    Renderer renderer_;
    void doInput();

public:
    Engine(hercules::Window &window);
    void DestroyEngine();
    void Loop();
    ~Engine();
};