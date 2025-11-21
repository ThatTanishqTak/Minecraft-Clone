#include "GameLayer.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "Engine/Events/Events.h"
#include "Engine/Input/Input.h"
#include "Engine/Renderer/Renderer.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

namespace
{
    // Clamp helper keeps the camera pitch within a safe range to avoid gimbal lock.
    float Clamp(float l_Value, float l_Min, float l_Max)
    {
        return std::max(l_Min, std::min(l_Value, l_Max));
    }
}

bool GameLayer::Initialize()
{
    if (m_IsInitialized)
    {
        // Avoid re-initializing if the layer is already active.
        return true;
    }

    // Prime camera state so view/projection matrices are valid before the first frame.
    m_Camera.SetPerspective(glm::radians(m_CameraFieldOfViewDegrees), 0.1f, 1000.0f);
    m_Camera.SetPosition(m_CameraPosition);
    m_Camera.SetLookAt(m_CameraPosition + glm::vec3{ 0.0f, 0.0f, -1.0f });
    m_Camera.SetUp(glm::vec3{ 0.0f, 1.0f, 0.0f });
    Engine::Renderer::SetCamera(m_Camera);

    // Load the block texture atlas and build chunk rendering helpers.
    m_TextureAtlas = std::make_unique<TextureAtlas>();
    if (!m_TextureAtlas->Load("Assets/Textures/Atlas.png", glm::ivec2{ 32, 32 }))
    {
        std::cout << "Failed to load texture atlas" << std::endl;
        return false;
    }

    // Map block faces to atlas tiles. The sample atlas is a 2x2 grid of solid colors.
    m_TextureAtlas->RegisterBlockFace(BlockId::Grass, BlockFace::Top, { 0, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Grass, BlockFace::Bottom, { 1, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Grass, BlockFace::North, { 1, 1 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Grass, BlockFace::South, { 1, 1 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Grass, BlockFace::East, { 1, 1 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Grass, BlockFace::West, { 1, 1 });

    m_TextureAtlas->RegisterBlockFace(BlockId::Dirt, BlockFace::Top, { 1, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Dirt, BlockFace::Bottom, { 1, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Dirt, BlockFace::North, { 1, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Dirt, BlockFace::South, { 1, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Dirt, BlockFace::East, { 1, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Dirt, BlockFace::West, { 1, 0 });

    m_TextureAtlas->RegisterBlockFace(BlockId::Stone, BlockFace::Top, { 0, 1 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Stone, BlockFace::Bottom, { 0, 1 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Stone, BlockFace::North, { 0, 1 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Stone, BlockFace::South, { 0, 1 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Stone, BlockFace::East, { 0, 1 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Stone, BlockFace::West, { 0, 1 });

    m_Chunk = std::make_unique<Chunk>(glm::ivec3{ 0 });
    m_ChunkMesher = std::make_unique<ChunkMesher>(*m_TextureAtlas);
    m_ChunkRenderer = std::make_unique<ChunkRenderer>();

    GenerateTestChunk();
    RefreshChunkMesh();

    // Bind example gameplay actions to specific inputs so Update() can consume them.
    Engine::Input::RegisterActionMapping("MoveForward", { GLFW_KEY_W });
    Engine::Input::RegisterActionMapping("Sprint", { GLFW_KEY_LEFT_SHIFT, GLFW_KEY_W });

    m_LastFrameTimeSeconds = glfwGetTime();

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

    // Track frame delta so movement scales with time instead of frame count.
    const double l_CurrentTimeSeconds = glfwGetTime();
    m_DeltaTimeSeconds = static_cast<float>(l_CurrentTimeSeconds - m_LastFrameTimeSeconds);
    m_LastFrameTimeSeconds = l_CurrentTimeSeconds;

    if (m_IsChunkDirty && m_ChunkMesher != nullptr)
    {
        // Rebuild the GPU mesh if blocks changed.
        RefreshChunkMesh();
    }

    // Demonstrate the new input API by polling both edge and hold state.
    const bool l_WasEscapePressed = Engine::Input::WasKeyPressedThisFrame(GLFW_KEY_ESCAPE);
    if (l_WasEscapePressed)
    {
        std::cout << "Escape was pressed this frame" << std::endl;
    }

    const bool l_IsSprinting = Engine::Input::IsActionDown("Sprint");
    if (l_IsSprinting)
    {
        std::cout << "Sprint combo is held" << std::endl;
    }

    const bool l_MoveTriggered = Engine::Input::WasActionPressedThisFrame("MoveForward");
    if (l_MoveTriggered)
    {
        std::cout << "MoveForward triggered this frame" << std::endl;
    }

    // Update camera orientation from mouse movement.
    const std::pair<float, float> l_MouseDelta = Engine::Input::GetMouseDelta();
    if (l_MouseDelta.first != 0.0f || l_MouseDelta.second != 0.0f)
    {
        m_CameraYawDegrees += l_MouseDelta.first * m_MouseSensitivity;
        m_CameraPitchDegrees -= l_MouseDelta.second * m_MouseSensitivity;
        m_CameraPitchDegrees = Clamp(m_CameraPitchDegrees, -89.0f, 89.0f);
    }

    // Calculate direction vectors from yaw/pitch so movement feels like an FPS camera.
    const glm::vec3 l_WorldUp{ 0.0f, 1.0f, 0.0f };
    glm::vec3 l_Forward{};
    l_Forward.x = cos(glm::radians(m_CameraYawDegrees)) * cos(glm::radians(m_CameraPitchDegrees));
    l_Forward.y = sin(glm::radians(m_CameraPitchDegrees));
    l_Forward.z = sin(glm::radians(m_CameraYawDegrees)) * cos(glm::radians(m_CameraPitchDegrees));
    l_Forward = glm::normalize(l_Forward);

    const glm::vec3 l_Right = glm::normalize(glm::cross(l_Forward, l_WorldUp));
    const glm::vec3 l_Up = glm::normalize(glm::cross(l_Right, l_Forward));

    float l_CurrentSpeed = m_CameraMoveSpeed;
    if (Engine::Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
    {
        l_CurrentSpeed *= 2.0f;
    }

    const float l_Velocity = l_CurrentSpeed * m_DeltaTimeSeconds;
    if (Engine::Input::IsKeyDown(GLFW_KEY_W))
    {
        m_CameraPosition += l_Forward * l_Velocity;
    }
    if (Engine::Input::IsKeyDown(GLFW_KEY_S))
    {
        m_CameraPosition -= l_Forward * l_Velocity;
    }
    if (Engine::Input::IsKeyDown(GLFW_KEY_A))
    {
        m_CameraPosition -= l_Right * l_Velocity;
    }
    if (Engine::Input::IsKeyDown(GLFW_KEY_D))
    {
        m_CameraPosition += l_Right * l_Velocity;
    }

    m_Camera.SetPosition(m_CameraPosition);
    m_Camera.SetLookAt(m_CameraPosition + l_Forward);
    m_Camera.SetUp(l_Up);

    // Sync the renderer's camera with updated transforms so the uniform buffer stays correct.
    Engine::Renderer::SetCamera(m_Camera);
}

void GameLayer::Render()
{
    if (!m_IsInitialized)
    {
        // Prevent rendering before the layer is ready.
        return;
    }

    if (m_ChunkRenderer != nullptr && m_TextureAtlas != nullptr)
    {
        // Draw the meshed chunk each frame.
        m_ChunkRenderer->Render(glm::mat4{ 1.0f }, m_TextureAtlas->GetTexture());
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

    m_ChunkRenderer.reset();
    m_ChunkMesher.reset();
    m_Chunk.reset();
    m_TextureAtlas.reset();
    m_IsInitialized = false;
}

void GameLayer::GenerateTestChunk()
{
    if (m_Chunk == nullptr)
    {
        return;
    }

    for (int l_Z = 0; l_Z < Chunk::CHUNK_SIZE; ++l_Z)
    {
        for (int l_X = 0; l_X < Chunk::CHUNK_SIZE; ++l_X)
        {
            // Use a light wave to vary terrain height across the chunk.
            const float l_Sample = std::sin(static_cast<float>(l_X) * 0.3f) + std::cos(static_cast<float>(l_Z) * 0.3f);
            const int l_Height = static_cast<int>(4 + l_Sample * 2.0f);

            for (int l_Y = 0; l_Y < Chunk::CHUNK_SIZE; ++l_Y)
            {
                BlockId l_Block = BlockId::Air;

                if (l_Y < l_Height - 2)
                {
                    l_Block = BlockId::Stone;
                }
                else if (l_Y < l_Height - 1)
                {
                    l_Block = BlockId::Dirt;
                }
                else if (l_Y == l_Height - 1)
                {
                    l_Block = BlockId::Grass;
                }

                m_Chunk->SetBlock(l_X, l_Y, l_Z, l_Block);
            }
        }
    }

    m_Chunk->RebuildVisibility();
    m_IsChunkDirty = true;
}

void GameLayer::RefreshChunkMesh()
{
    if (m_ChunkMesher == nullptr || m_ChunkRenderer == nullptr || m_Chunk == nullptr)
    {
        return;
    }

    const MeshedChunk l_MeshedChunk = m_ChunkMesher->Mesh(*m_Chunk);
    m_ChunkRenderer->UpdateMesh(l_MeshedChunk);
    m_IsChunkDirty = false;
}