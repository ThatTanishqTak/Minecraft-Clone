#include "Engine/Application.h"
#include "Engine/Core/Log.h"
#include "GameLayer.h"

#include <memory>

int main()
{
    Engine::Application l_Application;

    GAME_INFO("-------STARTING GAME-------");

    std::unique_ptr<GameLayer> l_GameLayer = std::make_unique<GameLayer>();

    l_Application.RegisterGameLayer(std::move(l_GameLayer));
    l_Application.Run();

    GAME_INFO("-------GAME SHUTDOWN COMPLETE-------");

    return 0;
}