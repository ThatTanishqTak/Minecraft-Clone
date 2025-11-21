#pragma once

#include "Engine/Application.h"
#include "Engine/Layer/Layer.h"
#include "Engine/Renderer/Camera.h"
#include "Engine/Renderer/Mesh.h"

#include "WorldGeneration/Chunk.h"
#include "WorldGeneration/ChunkMesher.h"
#include "WorldGeneration/ChunkRenderer.h"
#include "WorldGeneration/TextureAtlas.h"

#include <glm/glm.hpp>
#include <memory>

// GameLayer drives gameplay logic and rendering owned by the Game target.
class GameLayer : public Engine::Layer
{
public:
    GameLayer() = default;
    ~GameLayer() override = default;

    // Prepare gameplay systems and resources.
    bool Initialize() override;

    // Advance simulation for the current frame.
    void Update() override;

    // React to incoming engine events (keyboard, mouse, window, etc.).
    void OnEvent(const Engine::Event& event) override;

    // Draw the current frame.
    void Render() override;

    // Release resources when shutting down.
    void Shutdown() override;

private:
    void GenerateTestChunk();
    void RefreshChunkMesh();

    float CalculateSpawnHeightAboveTerrain() const;

private:
    bool m_IsInitialized = false;

    Engine::Camera m_Camera;
    glm::vec3 m_CameraPosition{ 0.0f, 10.5f, 5.0f };
    float m_CameraYawDegrees = -90.0f;
    float m_CameraPitchDegrees = 0.0f;
    float m_CameraFieldOfViewDegrees = 70.0f;
    float m_CameraMoveSpeed = 5.0f;
    float m_MouseSensitivity = 0.1f;

    double m_LastFrameTimeSeconds = 0.0;
    float m_DeltaTimeSeconds = 0.0f;

    std::unique_ptr<Chunk> m_Chunk;
    std::unique_ptr<TextureAtlas> m_TextureAtlas;
    std::unique_ptr<ChunkMesher> m_ChunkMesher;
    std::unique_ptr<ChunkRenderer> m_ChunkRenderer;
    bool m_IsChunkDirty = false;
};