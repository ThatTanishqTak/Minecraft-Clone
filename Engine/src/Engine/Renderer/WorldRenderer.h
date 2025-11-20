#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "Engine/Core/Core.h"
#include "Engine/Renderer/ChunkRenderComponent.h"
#include "Engine/Renderer/RenderQueue.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/World/ChunkMesher.h"

namespace Engine
{
    // Service responsible for world-specific rendering concerns (atlases, shaders, chunk meshes).
    class ENGINE_API WorldRenderer
    {
    public:
        WorldRenderer();
        ~WorldRenderer();

        // Build shared rendering resources such as the texture atlas and chunk shader.
        bool Initialize();

        // Release any GPU state and cached chunk render components.
        void Shutdown();

        // Build or rebuild a chunk's mesh and cache the GPU buffers for rendering.
        void BuildChunkMesh(const Chunk& chunk);

        // Queue the given chunk for rendering using the previously built mesh.
        void QueueChunkRender(const Chunk& chunk, RenderQueue& renderQueue) const;

    private:
        // Provide the default block definitions used to describe the world appearance.
        std::unordered_map<BlockType, BlockRenderInfo> BuildDefaultBlockTable() const;

        // Retrieve or create a render component for a specific chunk pointer.
        ChunkRenderComponent* GetOrCreateComponent(const Chunk& chunk);

        // Attach shared shader and texture resources to a render component.
        void ConfigureComponent(ChunkRenderComponent& component) const;

    private:
        bool m_IsInitialized = false;
        std::unordered_map<BlockType, BlockRenderInfo> m_BlockTable;
        std::shared_ptr<Shader> m_ChunkShader;
        std::unordered_map<const Chunk*, std::unique_ptr<ChunkRenderComponent>> m_ChunkComponents;
    };
}