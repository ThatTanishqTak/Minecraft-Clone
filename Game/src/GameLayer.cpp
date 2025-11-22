#include "GameLayer.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <chrono>
#include <limits>

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

    // Map each block to the correct tile coordinates for the provided 32x32 atlas grid. The
    // previous placeholder atlas used the first few tiles for every block, which left the new
    // atlas sampling the wrong pixels. Here we point the faces at the correct tiles taken from
    // the supplied atlas image so the generated chunk uses the intended artwork.
    struct BlockTextureDefinition
    {
        glm::ivec2 m_Top{};
        glm::ivec2 m_Bottom{};
        glm::ivec2 m_Side{};
    };

    const BlockTextureDefinition l_GrassTextures{ glm::ivec2{ 10, 0 }, glm::ivec2{ 17, 17 }, glm::ivec2{ 10, 1 } };
    const BlockTextureDefinition l_DirtTextures{ glm::ivec2{ 17, 17 }, glm::ivec2{ 17, 17 }, glm::ivec2{ 17, 17 } };
    const BlockTextureDefinition l_StoneTextures{ glm::ivec2{ 19, 14 }, glm::ivec2{ 19, 14 }, glm::ivec2{ 19, 14 } };

    const auto a_RegisterBlockTextures = [this](BlockId blockId, const BlockTextureDefinition& textures)
        {
            // Register top/bottom explicitly so the atlas lookup matches the requested face.
            m_TextureAtlas->RegisterBlockFace(blockId, BlockFace::Top, textures.m_Top);
            m_TextureAtlas->RegisterBlockFace(blockId, BlockFace::Bottom, textures.m_Bottom);

            // Sides all share a single tile to keep the definition concise.
            m_TextureAtlas->RegisterBlockFace(blockId, BlockFace::North, textures.m_Side);
            m_TextureAtlas->RegisterBlockFace(blockId, BlockFace::South, textures.m_Side);
            m_TextureAtlas->RegisterBlockFace(blockId, BlockFace::East, textures.m_Side);
            m_TextureAtlas->RegisterBlockFace(blockId, BlockFace::West, textures.m_Side);
        };

    a_RegisterBlockTextures(BlockId::Grass, l_GrassTextures);
    a_RegisterBlockTextures(BlockId::Dirt, l_DirtTextures);
    a_RegisterBlockTextures(BlockId::Stone, l_StoneTextures);

    // Build the chunk mesher now that the atlas is available so generated chunks can be rendered immediately.
    m_ChunkMesher = std::make_unique<ChunkMesher>(m_TextureAtlas.get());

    // Initialize world generation with deterministic parameters so terrain stays consistent between sessions.
    WorldGeneratorConfig l_WorldGeneratorConfig{};
    l_WorldGeneratorConfig.m_Seed = 20240601u;
    l_WorldGeneratorConfig.m_BaseHeight = 10;
    l_WorldGeneratorConfig.m_HeightAmplitude = 8;
    l_WorldGeneratorConfig.m_HeightFrequency = 0.04f;
    l_WorldGeneratorConfig.m_BiomeFrequency = 0.01f;
    l_WorldGeneratorConfig.m_BiomeStrength = 4.0f;
    l_WorldGeneratorConfig.m_CaveFrequency = 0.07f;
    l_WorldGeneratorConfig.m_CaveThreshold = 0.22f;
    l_WorldGeneratorConfig.m_EnableNoise = false; // Toggle to false for flat terrain when running deterministic threading tests.

    m_WorldGenerator = std::make_unique<WorldGenerator>(l_WorldGeneratorConfig);

    // Position the camera above the highest generated terrain so the player spawns in open space.
    const float l_SpawnHeight = CalculateSpawnHeightAboveTerrain();
    m_CameraPosition.y = l_SpawnHeight;
    GAME_TRACE("Calculated spawn height {:.7f} to clear generated terrain", l_SpawnHeight);

    // Prime camera state so view/projection matrices are valid before the first frame.
    m_Camera.SetPerspective(glm::radians(m_CameraFieldOfViewDegrees), 0.1f, 1000.0f);
    m_Camera.SetPosition(m_CameraPosition);
    m_Camera.SetLookAt(m_CameraPosition + glm::vec3{ 0.0f, 0.0f, -1.0f });
    m_Camera.SetUp(glm::vec3{ 0.0f, 1.0f, 0.0f });
    Engine::Renderer::SetCamera(m_Camera);
    GAME_TRACE("Camera primed for rendering with FOV {} degrees", m_CameraFieldOfViewDegrees);

    m_World = std::make_unique<World>(m_ChunkMesher.get(), m_TextureAtlas->GetTexture(), m_WorldGenerator.get());

    // Keep only nearby chunks alive so the renderer and memory footprint stay lean.
    constexpr int l_DefaultRenderDistance = 2;
    m_World->SetRenderDistance(l_DefaultRenderDistance);
    m_CurrentCameraChunkCoordinate = CalculateChunkCoordinate(m_CameraPosition);
    m_World->UpdateActiveChunks(m_CurrentCameraChunkCoordinate);
    m_World->RefreshChunkMeshes();
    GAME_TRACE("World streaming initialized around chunk ({}, {}, {})", m_CurrentCameraChunkCoordinate.x, m_CurrentCameraChunkCoordinate.y, m_CurrentCameraChunkCoordinate.z);

    // Bind gameplay actions to common movement keys so Update() can translate intent into motion.
    Engine::Input::RegisterActionMapping("MoveForward", { GLFW_KEY_W });
    Engine::Input::RegisterActionMapping("MoveBackward", { GLFW_KEY_S });
    Engine::Input::RegisterActionMapping("MoveLeft", { GLFW_KEY_A });
    Engine::Input::RegisterActionMapping("MoveRight", { GLFW_KEY_D });
    Engine::Input::RegisterActionMapping("Sprint", { GLFW_KEY_LEFT_SHIFT });
    GAME_TRACE("Input mappings registered for full movement set and Sprint");

    // Record the first frame timestamp so delta time stays accurate even if the GLFW timer is reset externally.
    m_LastFrameTimePoint = std::chrono::steady_clock::now();

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

    // Track frame delta so movement scales with time instead of frame count. Using std::chrono avoids
    // relying on GLFW's timer state, which can be reset by external calls and was leaving delta at 0.
    const std::chrono::steady_clock::time_point l_CurrentFrameTimePoint = std::chrono::steady_clock::now();
    const std::chrono::duration<float> l_FrameDelta = l_CurrentFrameTimePoint - m_LastFrameTimePoint;
    m_DeltaTimeSeconds = std::max(l_FrameDelta.count(), std::numeric_limits<float>::epsilon());
    m_LastFrameTimePoint = l_CurrentFrameTimePoint;

    //GAME_TRACE("Frame update started with delta time: {} seconds", m_DeltaTimeSeconds);

    // Demonstrate the new input API by polling both edge and hold state.
    const bool l_WasEscapePressed = Engine::Input::WasKeyPressedThisFrame(GLFW_KEY_ESCAPE);
    if (l_WasEscapePressed)
    {
        //GAME_INFO("Escape was pressed this frame");
    }

    // Check movement and sprint intent via the action system so behavior can evolve without touching keycodes.
    const bool l_IsSprinting = Engine::Input::IsActionDown("Sprint");
    const bool l_MoveForwardTriggered = Engine::Input::WasActionPressedThisFrame("MoveForward");
    const bool l_MoveBackwardTriggered = Engine::Input::WasActionPressedThisFrame("MoveBackward");
    const bool l_MoveLeftTriggered = Engine::Input::WasActionPressedThisFrame("MoveLeft");
    const bool l_MoveRightTriggered = Engine::Input::WasActionPressedThisFrame("MoveRight");

    // Future telemetry could use these triggers for analytics or tutorial prompts.
    if (l_MoveForwardTriggered || l_MoveBackwardTriggered || l_MoveLeftTriggered || l_MoveRightTriggered)
    {
        //GAME_INFO("Movement action triggered this frame");
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
    if (l_IsSprinting)
    {
        l_CurrentSpeed *= 2.0f;
    }

    // Resolve camera displacement from action state so all four directions are supported consistently.
    const float l_Velocity = l_CurrentSpeed * m_DeltaTimeSeconds;
    if (Engine::Input::IsActionDown("MoveForward"))
    {
        m_CameraPosition += l_Forward * l_Velocity;
    }
    if (Engine::Input::IsActionDown("MoveBackward"))
    {
        m_CameraPosition -= l_Forward * l_Velocity;
    }
    if (Engine::Input::IsActionDown("MoveLeft"))
    {
        m_CameraPosition -= l_Right * l_Velocity;
    }
    if (Engine::Input::IsActionDown("MoveRight"))
    {
        m_CameraPosition += l_Right * l_Velocity;
    }

    m_Camera.SetPosition(m_CameraPosition);
    m_Camera.SetLookAt(m_CameraPosition + l_Forward);
    m_Camera.SetUp(l_Up);

    // Sync the renderer's camera with updated transforms so the uniform buffer stays correct.
    Engine::Renderer::SetCamera(m_Camera);

    // Stream chunks when the camera crosses chunk boundaries so only nearby data is loaded.
    const glm::ivec3 l_NewCameraChunkCoordinate = CalculateChunkCoordinate(m_CameraPosition);
    if (l_NewCameraChunkCoordinate != m_CurrentCameraChunkCoordinate && m_World != nullptr)
    {
        m_CurrentCameraChunkCoordinate = l_NewCameraChunkCoordinate;
        m_World->UpdateActiveChunks(m_CurrentCameraChunkCoordinate);
    }

    if (m_World != nullptr)
    {
        m_World->RefreshChunkMeshes();
    }
}

void GameLayer::Render()
{
    if (!m_IsInitialized)
    {
        // Prevent rendering before the layer is ready.
        return;
    }

    if (m_World != nullptr)
    {
        for (const auto& it_ChunkEntry : m_World->GetActiveChunks())
        {
            const glm::ivec3& l_ChunkCoordinate = it_ChunkEntry.first;
            const World::ActiveChunk& l_ActiveChunk = it_ChunkEntry.second;

            if (l_ActiveChunk.m_Renderer != nullptr)
            {
                const glm::vec3 l_ChunkOffset = glm::vec3{ l_ChunkCoordinate } *static_cast<float>(Chunk::CHUNK_SIZE);
                const glm::mat4 l_Model = glm::translate(glm::mat4{ 1.0f }, l_ChunkOffset);
                l_ActiveChunk.m_Renderer->Render(l_Model);
            }
        }
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

    if (m_World != nullptr)
    {
        m_World->Shutdown();
    }

    m_World.reset();
    m_ChunkMesher.reset();
    m_IsInitialized = false;

    GAME_INFO("GameLayer shutdown complete");
}

float GameLayer::CalculateSpawnHeightAboveTerrain() const
{
    // Mirror the procedural terrain generation to compute the highest block near the origin and float above it.
    if (m_WorldGenerator == nullptr)
    {
        return static_cast<float>(Chunk::CHUNK_SIZE);
    }

    float l_MaxHeight = 0.0f;

    for (int l_Z = 0; l_Z < Chunk::CHUNK_SIZE; ++l_Z)
    {
        for (int l_X = 0; l_X < Chunk::CHUNK_SIZE; ++l_X)
        {
            const float l_Height = static_cast<float>(m_WorldGenerator->CalculateSurfaceHeight(l_X, l_Z));
            l_MaxHeight = std::max(l_MaxHeight, l_Height);
        }
    }

    // Add a buffer so the camera begins safely above the tallest peak.
    const float l_SpawnHeight = l_MaxHeight + 2.5f;

    return l_SpawnHeight;
}

glm::ivec3 GameLayer::CalculateChunkCoordinate(const glm::vec3& worldPosition) const
{
    // Floor divides by chunk size so negative coordinates correctly map to chunk indices.
    const float l_ChunkSize = static_cast<float>(Chunk::CHUNK_SIZE);
    const int l_ChunkX = static_cast<int>(std::floor(worldPosition.x / l_ChunkSize));
    const int l_ChunkY = static_cast<int>(std::floor(worldPosition.y / l_ChunkSize));
    const int l_ChunkZ = static_cast<int>(std::floor(worldPosition.z / l_ChunkSize));

    return glm::ivec3{ l_ChunkX, l_ChunkY, l_ChunkZ };
}