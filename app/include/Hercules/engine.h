#pragma once
#include "Hercules/renderer.h"
#include "Hercules/window.h"
#include <string>


class Engine
{
private:
    bool is_engine_running = false;
    hercules::Window window_;
    Renderer renderer_;

public:
    Engine(hercules::Window &window);
    void DestroyEngine();
    void Loop();
    ~Engine();
};