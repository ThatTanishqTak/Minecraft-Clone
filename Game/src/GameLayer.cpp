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

    //GAME_TRACE("GameLayer initialized");
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

    // TODO: Replace placeholder geometry with chunk mesh buffers once available.
    // TODO: Bind a texture atlas once asset loading is implemented.
    Engine::Renderer::DrawPlaceholderGeometry();
}

void GameLayer::Shutdown()
{
    if (!m_IsInitialized)
    {
        // Nothing to clean up if initialization never occurred.
        return;
    }

    //GAME_TRACE("GameLayer shutdown complete");
    m_IsInitialized = false;
}