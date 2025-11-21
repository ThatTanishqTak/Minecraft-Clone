#include "Engine/Application.h"
#include "Engine/Core/Log.h"
#include "GameLayer.h"

#include <memory>

int main()
{
    Engine::Application l_Application;

    GAME_INFO("Starting game entry point");

    // Transfer ownership of the gameplay layer to the engine so it can manage the lifecycle.
    std::unique_ptr<GameLayer> l_GameLayer = std::make_unique<GameLayer>();

    // Allow the engine to drive the game layer lifecycle without the game calling Initialize or Shutdown directly.
    l_Application.RegisterGameLayer(std::move(l_GameLayer));
    l_Application.Run();

    GAME_INFO("Game shutdown complete");

    return 0;
}