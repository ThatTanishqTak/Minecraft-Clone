#pragma once

#include <unordered_map>
#include <unordered_set>
#include <queue>
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

    // Now processes a limited number of mesh rebuild jobs per frame instead of
    // synchronously remeshing every dirty chunk.
    void RefreshChunkMeshes();

    void MarkChunkDirty(const glm::ivec3& chunkCoordinate);

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
    std::unordered_map<glm::ivec3, std::shared_ptr<Engine::Mesh>, IVec3Hasher> m_MeshPool;

    // Queue of chunk coordinates that need their mesh rebuilt.
    std::queue<glm::ivec3> m_MeshRebuildQueue;

    // Tracks which chunk coordinates are already queued so we do not enqueue duplicates.
    std::unordered_set<glm::ivec3, IVec3Hasher> m_PendingMeshUpdates;
};