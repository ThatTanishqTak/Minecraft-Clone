#include "GameLayer.h"

#include <iostream>
#include <vector>

#include "Engine/Renderer/Renderer.h"

bool GameLayer::Initialize()
{
    if (m_IsInitialized)
    {
        // Avoid re-initializing if the layer is already active.
        return true;
    }

    m_WorldRenderer = std::make_unique<Engine::WorldRenderer>();
    if (m_WorldRenderer == nullptr || !m_WorldRenderer->Initialize())
    {
        //ENGINE_ERROR("Failed to initialize world rendering service");
        return false;
    }

    // Populate a basic terrain slice so the atlas coordinates can be visualized.
    for (int x = 0; x < Engine::Chunk::s_SizeX; ++x)
    {
        for (int z = 0; z < Engine::Chunk::s_SizeZ; ++z)
        {
            m_Chunk.SetBlock(x, 0, z, Engine::BlockType::Stone);
            m_Chunk.SetBlock(x, 1, z, Engine::BlockType::Dirt);
            m_Chunk.SetBlock(x, 2, z, Engine::BlockType::Dirt);
            m_Chunk.SetBlock(x, 3, z, Engine::BlockType::Grass);
        }
    }

    m_WorldRenderer->BuildChunkMesh(m_Chunk);

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

    Engine::RenderQueue* l_Queue = Engine::Renderer::GetRenderQueue();
    if (l_Queue != nullptr && m_WorldRenderer != nullptr)
    {
        m_WorldRenderer->QueueChunkRender(m_Chunk, *l_Queue);
    }
}

void GameLayer::Shutdown()
{
    if (!m_IsInitialized)
    {
        // Nothing to clean up if initialization never occurred.
        return;
    }

    if (m_WorldRenderer != nullptr)
    {
        m_WorldRenderer->Shutdown();
        m_WorldRenderer.reset();
    }

    //GAME_TRACE("GameLayer shutdown complete");
    m_IsInitialized = false;
}