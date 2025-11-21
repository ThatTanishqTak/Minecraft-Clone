#include "GameLayer.h"

#include <algorithm>
#include <cmath>
#include <filesystem>

#include "Engine/Events/Events.h"
#include "Engine/Core/Log.h"
#include "Engine/Input/Input.h"
#include "Engine/Renderer/Renderer.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

namespace
{
    // Clamp helper keeps the camera pitch within a safe range to avoid gimbal lock.
    float Clamp(float value, float minimum, float maximum)
    {
        return std::max(minimum, std::min(value, maximum));
    }
}

bool GameLayer::Initialize()
{
    if (m_IsInitialized)
    {
        // Avoid duplicating setup and note the unexpected call path.
        GAME_WARN("GameLayer::Initialize called while already initialized");

        // Avoid re-initializing if the layer is already active.
        return true;
    }

    GAME_INFO("GameLayer initialization starting");

    // Load the block atlas so UVs can be generated during meshing and sampled at render time.
    const std::filesystem::path l_AtlasPath = std::filesystem::path("Assets/Textures/Atlas.png");
    const glm::ivec2 l_AtlasTileSize{ 16, 16 };

    m_TextureAtlas = std::make_unique<TextureAtlas>();
    if (!m_TextureAtlas->Load(l_AtlasPath.string(), l_AtlasTileSize))
    {
        GAME_ERROR("GameLayer failed to load texture atlas at {}", l_AtlasPath.string());

        return false;
    }

    // Register UVs for each block face so the mesher can emit correct coordinates.
    m_TextureAtlas->RegisterBlockFace(BlockId::Grass, BlockFace::Top, glm::ivec2{ 0, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Grass, BlockFace::Bottom, glm::ivec2{ 2, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Grass, BlockFace::North, glm::ivec2{ 1, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Grass, BlockFace::South, glm::ivec2{ 1, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Grass, BlockFace::East, glm::ivec2{ 1, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Grass, BlockFace::West, glm::ivec2{ 1, 0 });

    m_TextureAtlas->RegisterBlockFace(BlockId::Dirt, BlockFace::Top, glm::ivec2{ 2, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Dirt, BlockFace::Bottom, glm::ivec2{ 2, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Dirt, BlockFace::North, glm::ivec2{ 2, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Dirt, BlockFace::South, glm::ivec2{ 2, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Dirt, BlockFace::East, glm::ivec2{ 2, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Dirt, BlockFace::West, glm::ivec2{ 2, 0 });

    m_TextureAtlas->RegisterBlockFace(BlockId::Stone, BlockFace::Top, glm::ivec2{ 3, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Stone, BlockFace::Bottom, glm::ivec2{ 3, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Stone, BlockFace::North, glm::ivec2{ 3, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Stone, BlockFace::South, glm::ivec2{ 3, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Stone, BlockFace::East, glm::ivec2{ 3, 0 });
    m_TextureAtlas->RegisterBlockFace(BlockId::Stone, BlockFace::West, glm::ivec2{ 3, 0 });

    // Position the camera above the highest generated terrain so the player spawns in open space.
    const float l_SpawnHeight = CalculateSpawnHeightAboveTerrain();
    m_CameraPosition.y = l_SpawnHeight;
    GAME_TRACE("Calculated spawn height {:.2f} to clear generated terrain", l_SpawnHeight);

    // Prime camera state so view/projection matrices are valid before the first frame.
    m_Camera.SetPerspective(glm::radians(m_CameraFieldOfViewDegrees), 0.1f, 1000.0f);
    m_Camera.SetPosition(m_CameraPosition);
    m_Camera.SetLookAt(m_CameraPosition + glm::vec3{ 0.0f, 0.0f, -1.0f });
    m_Camera.SetUp(glm::vec3{ 0.0f, 1.0f, 0.0f });
    Engine::Renderer::SetCamera(m_Camera);
    GAME_TRACE("Camera primed for rendering with FOV {} degrees", m_CameraFieldOfViewDegrees);

    m_Chunk = std::make_unique<Chunk>(glm::ivec3{ 0 });
    m_ChunkMesher = std::make_unique<ChunkMesher>(m_TextureAtlas.get());
    m_ChunkRenderer = std::make_unique<ChunkRenderer>();
    m_ChunkRenderer->SetTexture(m_TextureAtlas->GetTexture());
    GAME_TRACE("Chunk systems created and ready");

    GenerateTestChunk();
    RefreshChunkMesh();

    // Bind example gameplay actions to specific inputs so Update() can consume them.
    Engine::Input::RegisterActionMapping("MoveForward", { GLFW_KEY_W });
    Engine::Input::RegisterActionMapping("Sprint", { GLFW_KEY_LEFT_SHIFT, GLFW_KEY_W });
    GAME_TRACE("Input mappings registered for MoveForward and Sprint");

    m_LastFrameTimeSeconds = glfwGetTime();

    m_IsInitialized = true;

    GAME_INFO("GameLayer initialization completed successfully");

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

    //GAME_TRACE("Frame update started with delta time: {} seconds", m_DeltaTimeSeconds);

    if (m_IsChunkDirty && m_ChunkMesher != nullptr)
    {
        // Rebuild the GPU mesh if blocks changed.
        //GAME_TRACE("Chunk marked dirty, rebuilding mesh");
        RefreshChunkMesh();
    }

    // Demonstrate the new input API by polling both edge and hold state.
    const bool l_WasEscapePressed = Engine::Input::WasKeyPressedThisFrame(GLFW_KEY_ESCAPE);
    if (l_WasEscapePressed)
    {
        //GAME_INFO("Escape was pressed this frame");
    }

    const bool l_IsSprinting = Engine::Input::IsActionDown("Sprint");
    if (l_IsSprinting)
    {
        //GAME_INFO("Sprint combo is held");
    }

    const bool l_MoveTriggered = Engine::Input::WasActionPressedThisFrame("MoveForward");
    if (l_MoveTriggered)
    {
        //GAME_INFO("MoveForward triggered this frame");
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

    if (m_ChunkRenderer != nullptr)
    {
        // Draw the meshed chunk each frame.
        m_ChunkRenderer->Render(glm::mat4{ 1.0f });
        //GAME_TRACE("Rendered chunk with current texture atlas");
    }
}

void GameLayer::OnEvent(const Engine::Event& event)
{
    // Future event handling can react to inputs; currently this layer logs handled events when needed.
    GAME_TRACE("GameLayer received event type {}", static_cast<int>(event.GetEventType()));
}

void GameLayer::Shutdown()
{
    if (!m_IsInitialized)
    {
        // Nothing to clean up if initialization never occurred.
        return;
    }

    GAME_INFO("Shutting down GameLayer and releasing resources");

    m_ChunkRenderer.reset();
    m_ChunkMesher.reset();
    m_Chunk.reset();
    m_IsInitialized = false;

    GAME_INFO("GameLayer shutdown complete");
}

void GameLayer::GenerateTestChunk()
{
    if (m_Chunk == nullptr)
    {
        return;
    }

    GAME_TRACE("Generating test chunk data");

    for (int z = 0; z < Chunk::CHUNK_SIZE; ++z)
    {
        for (int x = 0; x < Chunk::CHUNK_SIZE; ++x)
        {
            // Use a light wave to vary terrain height across the chunk.
            const float l_Sample = std::sin(static_cast<float>(x) * 0.3f) + std::cos(static_cast<float>(z) * 0.3f);
            const int l_Height = static_cast<int>(4 + l_Sample * 2.0f);

            for (int y = 0; y < Chunk::CHUNK_SIZE; ++y)
            {
                BlockId l_Block = BlockId::Air;

                if (y < l_Height - 2)
                {
                    l_Block = BlockId::Stone;
                }
                else if (y < l_Height - 1)
                {
                    l_Block = BlockId::Dirt;
                }
                else if (y == l_Height - 1)
                {
                    l_Block = BlockId::Grass;
                }

                m_Chunk->SetBlock(x, y, z, l_Block);
            }
        }
    }

    m_Chunk->RebuildVisibility();
    m_IsChunkDirty = true;

    GAME_TRACE("Chunk data generated and marked dirty for meshing");
}

float GameLayer::CalculateSpawnHeightAboveTerrain() const
{
    // Mirror the test terrain generation to compute the highest block and float above it.
    float l_MaxHeight = 0.0f;

    for (int z = 0; z < Chunk::CHUNK_SIZE; ++z)
    {
        for (int x = 0; x < Chunk::CHUNK_SIZE; ++x)
        {
            const float l_Sample = std::sin(static_cast<float>(x) * 0.3f) + std::cos(static_cast<float>(z) * 0.3f);
            const float l_Height = static_cast<float>(4 + l_Sample * 2.0f);

            l_MaxHeight = std::max(l_MaxHeight, l_Height);
        }
    }

    // Add a buffer so the camera begins safely above the tallest peak.
    const float l_SpawnHeight = l_MaxHeight + 2.5f;

    return l_SpawnHeight;
}

void GameLayer::RefreshChunkMesh()
{
    if (m_ChunkMesher == nullptr || m_ChunkRenderer == nullptr || m_Chunk == nullptr)
    {
        //GAME_WARN("RefreshChunkMesh called with missing components");

        return;
    }

    const MeshedChunk l_MeshedChunk = m_ChunkMesher->Mesh(*m_Chunk);
    //GAME_TRACE("Meshed chunk produced {} vertices and {} indices", l_MeshedChunk.m_Vertices.size(), l_MeshedChunk.m_Indices.size());

    m_ChunkRenderer->UpdateMesh(l_MeshedChunk);
    m_IsChunkDirty = false;

    //GAME_INFO("Chunk mesh refreshed and uploaded to renderer");
}