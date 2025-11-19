#include "GameLayer.h"

#include <iostream>

GameLayer::~GameLayer()
{
    // Ensure resources are released when the layer leaves scope.
    Shutdown();
}

bool GameLayer::Initialize()
{
    if (m_IsInitialized)
    {
        // Avoid re-initializing if the layer is already active.
        return true;
    }

    std::cout << "GameLayer initialized" << std::endl;
    m_IsInitialized = true;

    return m_IsInitialized;
}

void GameLayer::Update()
{
    if (!m_IsInitialized)
    {
        // Skip update work when initialization has not succeeded.
        return;
    }

    // TODO: Insert per-frame game logic here.
}

void GameLayer::Render()
{
    if (!m_IsInitialized)
    {
        // Prevent rendering before the layer is ready.
        return;
    }

    // TODO: Insert rendering commands (e.g., draw world, UI).
}

void GameLayer::Shutdown()
{
    if (!m_IsInitialized)
    {
        // Nothing to clean up if initialization never occurred.
        return;
    }

    std::cout << "GameLayer shutdown complete" << std::endl;
    m_IsInitialized = false;
}