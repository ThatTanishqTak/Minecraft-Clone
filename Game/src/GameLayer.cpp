#include "GameLayer.h"

#include <iostream>
#include <vector>

#include "Engine/Renderer/Renderer.h"
#include "Engine/Events/Events.h"

bool GameLayer::Initialize()
{
    if (m_IsInitialized)
    {
        // Avoid re-initializing if the layer is already active.
        return true;
    }

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
}

void GameLayer::OnEvent(const Engine::Event& event)
{
    // Future event handling can react to inputs; currently this layer logs handled events when needed.
    (void)event;
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