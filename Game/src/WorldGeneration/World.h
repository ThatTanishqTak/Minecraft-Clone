#pragma once

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <glm/glm.hpp>

#include "Chunk.h"
#include "ChunkMesher.h"
#include "ChunkRenderer.h"
#include "WorldGenerator.h"

namespace Engine
{
    class Texture2D;
}

// Streams chunks around a focal point so the renderer only touches nearby terrain.
class World
{
public:
    struct ActiveChunk
    {
        std::unique_ptr<Chunk> m_Chunk;
        std::unique_ptr<ChunkRenderer> m_Renderer;
        bool m_IsDirty = false;
    };

    struct IVec3Hasher
    {
        size_t operator()(const glm::ivec3& key) const noexcept;
    };

public:
    World(ChunkMesher* chunkMesher, const Engine::Texture2D* texture, const WorldGenerator* worldGenerator);

    void SetRenderDistance(int renderDistance);
    void UpdateActiveChunks(const glm::ivec3& centerChunkCoordinate);
    void RefreshChunkMeshes();

    const std::unordered_map<glm::ivec3, ActiveChunk, IVec3Hasher>& GetActiveChunks() const { return m_ActiveChunks; }

    void Shutdown();

private:
    void CreateChunkIfMissing(const glm::ivec3& chunkCoordinate);
    void PopulateChunkBlocks(Chunk& chunk) const;
    void MeshChunkIfDirty(ActiveChunk& chunkData);

private:
    ChunkMesher* m_ChunkMesher = nullptr;
    const Engine::Texture2D* m_Texture = nullptr;
    const WorldGenerator* m_WorldGenerator = nullptr;
    int m_RenderDistance = 2;
    std::unordered_map<glm::ivec3, ActiveChunk, IVec3Hasher> m_ActiveChunks;
};