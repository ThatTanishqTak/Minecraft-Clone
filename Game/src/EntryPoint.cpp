#include "Engine/Application.h"
#include "GameLayer.h"

int main()
{
    GameLayer l_GameLayer{};

    // Initialize the game layer before handing it to the engine.
    if (!l_GameLayer.Initialize())
    {
        return -1;
    }

    Engine::Application l_Application;

    // Allow the engine to drive the game layer lifecycle.
    l_Application.RegisterGameLayer(&l_GameLayer);
    l_Application.Run();

    // Clean up resources on shutdown to keep ownership explicit.
    l_GameLayer.Shutdown();

    return 0;
}