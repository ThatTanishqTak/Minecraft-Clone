#include "GameLayer.h"

#include <iostream>
#include <vector>

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

    // Build a compact atlas covering the default block set.
    std::vector<Engine::BlockTextureDefinition> l_Definitions = {};

    Engine::BlockTextureDefinition l_Dirt = {};
    l_Dirt.Block = Engine::BlockType::Dirt;
    l_Dirt.FaceTextures.fill("dirt");
    l_Definitions.push_back(l_Dirt);

    Engine::BlockTextureDefinition l_Grass = {};
    l_Grass.Block = Engine::BlockType::Grass;
    l_Grass.FaceTextures = { "grass_side", "grass_side", "grass_top", "dirt", "grass_side", "grass_side" };
    l_Definitions.push_back(l_Grass);

    Engine::BlockTextureDefinition l_Stone = {};
    l_Stone.Block = Engine::BlockType::Stone;
    l_Stone.FaceTextures.fill("stone");
    l_Stone.MaterialFlags = 0.2f;
    l_Definitions.push_back(l_Stone);

    m_TextureAtlas = std::make_unique<Engine::TextureAtlas>();
    if (!m_TextureAtlas->BuildFromDefinitions("Game/assets/textures/blocks", l_Definitions, 16))
    {
        ENGINE_ERROR("Failed to build texture atlas");
        return false;
    }

    // Simple shader that converts atlas coordinates into final texture lookups.
    const char* l_VertexSource = R"(
        #version 430 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aLocalUV;
        layout (location = 3) in vec2 aAtlasMin;
        layout (location = 4) in vec2 aAtlasMax;
        layout (location = 5) in float aMaterialFlags;

        out vec2 vUV;
        out float vMaterialFlags;
        out vec3 vNormal;

        void main()
        {
            vec2 l_UVRange = aAtlasMax - aAtlasMin;
            vUV = aAtlasMin + l_UVRange * aLocalUV;
            vMaterialFlags = aMaterialFlags;
            vNormal = aNormal;
            gl_Position = vec4(aPos / 8.0 - vec3(1.0), 1.0);
        }
    )";

    const char* l_FragmentSource = R"(
        #version 430 core
        in vec2 vUV;
        in float vMaterialFlags;
        in vec3 vNormal;
        out vec4 FragColor;

        uniform sampler2D u_AtlasTexture;

        void main()
        {
            vec4 l_Albedo = texture(u_AtlasTexture, vUV);
            float l_Unlit = clamp(vMaterialFlags, 0.0, 1.0);
            float l_Light = max(dot(normalize(vNormal), normalize(vec3(0.3, 1.0, 0.5))), 0.25);
            FragColor = vec4(l_Albedo.rgb * mix(l_Light, 1.0, l_Unlit), l_Albedo.a);
        }
    )";

    m_ChunkShader = std::make_shared<Engine::Shader>(l_VertexSource, l_FragmentSource);
    if (m_ChunkShader == nullptr || !m_ChunkShader->IsValid())
    {
        ENGINE_ERROR("Failed to compile chunk shader");
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

    Engine::ChunkMesh l_Mesh = Engine::ChunkMesher::GenerateMesh(m_Chunk, m_TextureAtlas->GetBlockTable());
    m_RenderComponent.SetShader(m_ChunkShader);
    m_RenderComponent.SetTexture(m_TextureAtlas->GetTextureId());
    m_RenderComponent.UpdateMesh(l_Mesh);

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
    if (l_Queue != nullptr)
    {
        m_RenderComponent.Enqueue(*l_Queue);
    }
}

void GameLayer::Shutdown()
{
    if (!m_IsInitialized)
    {
        // Nothing to clean up if initialization never occurred.
        return;
    }

    m_TextureAtlas.reset();
    m_ChunkShader.reset();
    //GAME_TRACE("GameLayer shutdown complete");
    m_IsInitialized = false;
}