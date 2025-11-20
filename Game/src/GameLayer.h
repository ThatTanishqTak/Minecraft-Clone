#pragma once

#include "Engine/Core/Log.h"
#include "Engine/Application.h"
#include "Engine/Layer/Layer.h"
#include "Engine/Renderer/ChunkRenderComponent.h"
#include "Engine/Renderer/TextureAtlas.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/World/ChunkMesher.h"

// GameLayer drives gameplay logic and rendering owned by the Game target.
class GameLayer : public Engine::Layer
{
public:
    GameLayer() = default;
    ~GameLayer();

    // Prepare gameplay systems and resources.
    bool Initialize() override;

    // Advance simulation for the current frame.
    void Update() override;

    // Draw the current frame.
    void Render() override;

    // Release resources when shutting down.
    void Shutdown() override;

private:
    bool m_IsInitialized = false;
    Engine::Chunk m_Chunk;
    Engine::ChunkRenderComponent m_RenderComponent;
    std::unique_ptr<Engine::TextureAtlas> m_TextureAtlas;
    std::shared_ptr<Engine::Shader> m_ChunkShader;
};