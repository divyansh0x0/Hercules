// for initializing and shutdown functions
#define SDL_MAIN_HANDLED
#include <logger.h>
#include <Engine/engine.h>

int main(int argc, char *argv[])
{
    Engine engine(1000,800);

    engine.showWindow();


    engine.loop();
    return 0;
}
