#include "GameLayer.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <chrono>
#include <limits>
#include <array>

#include "Engine/Events/Events.h"
#include "Engine/Core/Log.h"
#include "Engine/Input/Input.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

bool GameLayer::Initialize()
{
    return true;
}

void GameLayer::Update()
{

}

void GameLayer::Render()
{

}


void GameLayer::OnEvent(const Engine::Event& event)
{

}

void GameLayer::Shutdown()
{
    GAME_INFO("GameLayer shutdown complete");
}