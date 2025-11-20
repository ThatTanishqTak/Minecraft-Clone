#include "Engine/Renderer/WorldRenderer.h"

#include <iostream>

namespace Engine
{
    WorldRenderingService::WorldRenderingService() = default;

    WorldRenderingService::~WorldRenderingService()
    {
        // Ensure GPU resources are reclaimed if the service goes out of scope unexpectedly.
        Shutdown();
    }

    bool WorldRenderingService::Initialize()
    {
        if (m_IsInitialized)
        {
            // Avoid rebuilding resources if the service is reused.
            return true;
        }

        m_TextureAtlas = std::make_unique<TextureAtlas>();
        const std::vector<BlockTextureDefinition> l_Definitions = BuildDefaultBlockDefinitions();

        if (!m_TextureAtlas->BuildFromDefinitions("Game/assets/textures/blocks", l_Definitions, 16))
        {
            std::cout << "Failed to build texture atlas" << std::endl;
            return false;
        }

        // Shader converts atlas UV ranges into normalized lookups and applies a basic light term.
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

        m_ChunkShader = std::make_shared<Shader>(l_VertexSource, l_FragmentSource);
        if (m_ChunkShader == nullptr || !m_ChunkShader->IsValid())
        {
            std::cout << "Failed to compile chunk shader" << std::endl;
            return false;
        }

        m_IsInitialized = true;
        return true;
    }

    void WorldRenderingService::Shutdown()
    {
        // Clear per-chunk GPU components first to release buffers before globals.
        m_ChunkComponents.clear();
        m_TextureAtlas.reset();
        m_ChunkShader.reset();
        m_IsInitialized = false;
    }

    void WorldRenderingService::BuildChunkMesh(const Chunk& chunk)
    {
        if (!m_IsInitialized || m_TextureAtlas == nullptr)
        {
            // Initialization must run before mesh generation can occur.
            return;
        }

        ChunkRenderComponent* l_Component = GetOrCreateComponent(chunk);
        if (l_Component == nullptr)
        {
            return;
        }

        ChunkMesh l_Mesh = ChunkMesher::GenerateMesh(chunk, m_TextureAtlas->GetBlockTable());
        ConfigureComponent(*l_Component);
        l_Component->UpdateMesh(l_Mesh);
    }

    void WorldRenderingService::QueueChunkRender(const Chunk& chunk, RenderQueue& renderQueue) const
    {
        auto l_Iterator = m_ChunkComponents.find(&chunk);
        if (l_Iterator == m_ChunkComponents.end())
        {
            // Skip queueing if the chunk has not been meshed yet.
            return;
        }

        l_Iterator->second->Enqueue(renderQueue);
    }

    std::vector<BlockTextureDefinition> WorldRenderingService::BuildDefaultBlockDefinitions() const
    {
        // Basic block set used by the initial sandbox world.
        std::vector<BlockTextureDefinition> l_Definitions = {};

        BlockTextureDefinition l_Dirt = {};
        l_Dirt.Block = BlockType::Dirt;
        l_Dirt.FaceTextures.fill("dirt");
        l_Definitions.push_back(l_Dirt);

        BlockTextureDefinition l_Grass = {};
        l_Grass.Block = BlockType::Grass;
        l_Grass.FaceTextures = { "grass_side", "grass_side", "grass_top", "dirt", "grass_side", "grass_side" };
        l_Definitions.push_back(l_Grass);

        BlockTextureDefinition l_Stone = {};
        l_Stone.Block = BlockType::Stone;
        l_Stone.FaceTextures.fill("stone");
        l_Stone.MaterialFlags = 0.2f;
        l_Definitions.push_back(l_Stone);

        return l_Definitions;
    }

    ChunkRenderComponent* WorldRenderingService::GetOrCreateComponent(const Chunk& chunk)
    {
        auto l_Iterator = m_ChunkComponents.find(&chunk);
        if (l_Iterator != m_ChunkComponents.end())
        {
            return l_Iterator->second.get();
        }

        // Create and store a render component keyed by the chunk address.
        std::unique_ptr<ChunkRenderComponent> l_NewComponent = std::make_unique<ChunkRenderComponent>();
        ChunkRenderComponent* l_ComponentPtr = l_NewComponent.get();
        m_ChunkComponents[&chunk] = std::move(l_NewComponent);

        return l_ComponentPtr;
    }

    void WorldRenderingService::ConfigureComponent(ChunkRenderComponent& component) const
    {
        if (m_TextureAtlas == nullptr || m_ChunkShader == nullptr)
        {
            return;
        }

        // Share the same shader and atlas across all chunk components.
        component.SetShader(m_ChunkShader);
        component.SetTexture(m_TextureAtlas->GetTextureId());
    }
}