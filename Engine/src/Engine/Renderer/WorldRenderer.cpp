#include "Engine/Renderer/WorldRenderer.h"

#include <iostream>

namespace Engine
{
    WorldRenderer::WorldRenderer() = default;

    WorldRenderer::~WorldRenderer()
    {
        // Ensure GPU resources are reclaimed if the service goes out of scope unexpectedly.
        Shutdown();
    }

    bool WorldRenderer::Initialize()
    {
        if (m_IsInitialized)
        {
            // Avoid rebuilding resources if the service is reused.
            return true;
        }

        m_BlockTable = BuildDefaultBlockTable();

        // Shader passes per-vertex solid colors through a simple directional light term.
        const char* l_VertexSource = R"(
            #version 430 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;
            layout (location = 2) in vec3 aColor;
            layout (location = 3) in float aMaterialFlags;

            out vec3 vColor;
            out float vMaterialFlags;
            out vec3 vNormal;

            void main()
            {
                vColor = aColor;
                vMaterialFlags = aMaterialFlags;
                vNormal = aNormal;
                gl_Position = vec4(aPos / 8.0 - vec3(1.0), 1.0);
            }
        )";

        const char* l_FragmentSource = R"(
            #version 430 core
            in vec3 vColor;
            in float vMaterialFlags;
            in vec3 vNormal;
            out vec4 FragColor;

            void main()
            {
                vec3 l_Albedo = vColor;
                float l_Unlit = clamp(vMaterialFlags, 0.0, 1.0);
                float l_Light = max(dot(normalize(vNormal), normalize(vec3(0.3, 1.0, 0.5))), 0.25);
                FragColor = vec4(l_Albedo * mix(l_Light, 1.0, l_Unlit), 1.0);
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

    void WorldRenderer::Shutdown()
    {
        // Clear per-chunk GPU components first to release buffers before globals.
        m_ChunkComponents.clear();
        m_BlockTable.clear();
        m_ChunkShader.reset();
        m_IsInitialized = false;
    }

    void WorldRenderer::BuildChunkMesh(const Chunk& chunk)
    {
        if (!m_IsInitialized)
        {
            // Initialization must run before mesh generation can occur.
            return;
        }

        ChunkRenderComponent* l_Component = GetOrCreateComponent(chunk);
        if (l_Component == nullptr)
        {
            return;
        }

        ChunkMesh l_Mesh = ChunkMesher::GenerateMesh(chunk, m_BlockTable);
        ConfigureComponent(*l_Component);
        l_Component->UpdateMesh(l_Mesh);
    }

    void WorldRenderer::QueueChunkRender(const Chunk& chunk, RenderQueue& renderQueue) const
    {
        auto l_Iterator = m_ChunkComponents.find(&chunk);
        if (l_Iterator == m_ChunkComponents.end())
        {
            // Skip queueing if the chunk has not been meshed yet.
            return;
        }

        l_Iterator->second->Enqueue(renderQueue);
    }

    std::unordered_map<BlockType, BlockRenderInfo> WorldRenderer::BuildDefaultBlockTable() const
    {
        // Basic block set used by the initial sandbox world.
        std::unordered_map<BlockType, BlockRenderInfo> l_Definitions = {};

        BlockRenderInfo l_Dirt = {};
        l_Dirt.IsOpaque = true;
        l_Dirt.Color = glm::vec3(0.47f, 0.33f, 0.24f);
        l_Definitions[BlockType::Dirt] = l_Dirt;

        BlockRenderInfo l_Grass = {};
        l_Grass.IsOpaque = true;
        l_Grass.Color = glm::vec3(0.38f, 0.62f, 0.29f);
        l_Definitions[BlockType::Grass] = l_Grass;

        BlockRenderInfo l_Stone = {};
        l_Stone.IsOpaque = true;
        l_Stone.Color = glm::vec3(0.55f, 0.56f, 0.60f);
        l_Stone.MaterialFlags = 0.2f;
        l_Definitions[BlockType::Stone] = l_Stone;

        return l_Definitions;
    }

    ChunkRenderComponent* WorldRenderer::GetOrCreateComponent(const Chunk& chunk)
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

    void WorldRenderer::ConfigureComponent(ChunkRenderComponent& component) const
    {
        if (m_ChunkShader == nullptr)
        {
            return;
        }

        // Share the same shader across all chunk components.
        component.SetShader(m_ChunkShader);
    }
}