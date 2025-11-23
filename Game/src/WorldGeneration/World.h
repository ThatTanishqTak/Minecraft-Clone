#pragma once

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <queue>
#include <thread>
#include <atomic>

#include <glm/glm.hpp>

#include "Chunk.h"
#include "ChunkMesher.h"
#include "ChunkRenderer.h"
#include "WorldGenerator.h"
#include "ThreadSafeQueue.h"  // For cross-thread chunk generation handoff. 

namespace Engine
{
    class Texture2D;
    class Mesh;
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
    ~World();

    void SetRenderDistance(int renderDistance);

    // Called every frame with the camera chunk coordinate.
    void UpdateActiveChunks(const glm::ivec3& centerChunkCoordinate);

    // Process a limited number of mesh rebuilds per frame.
    void RefreshChunkMeshes();

    // Mark a chunk as dirty so it is re-meshed via the per-frame queue.
    void MarkChunkDirty(const glm::ivec3& chunkCoordinate);

    const std::unordered_map<glm::ivec3, ActiveChunk, IVec3Hasher>& GetActiveChunks() const { return m_ActiveChunks; }

    void Shutdown();

private:
    void CreateChunkIfMissing(const glm::ivec3& chunkCoordinate); // kept for possible future use
    void PopulateChunkBlocks(Chunk& chunk) const;
    void MeshChunkIfDirty(ActiveChunk& chunkData);

    // Background generation
    void StartGenerationWorker();
    void StopGenerationWorker();
    void PumpGenerationResults();

private:
    ChunkMesher* m_ChunkMesher = nullptr;
    const Engine::Texture2D* m_Texture = nullptr;
    const WorldGenerator* m_WorldGenerator = nullptr;

    int m_RenderDistance = 2;

    std::unordered_map<glm::ivec3, ActiveChunk, IVec3Hasher> m_ActiveChunks;
    std::unordered_map<glm::ivec3, std::shared_ptr<Engine::Mesh>, IVec3Hasher> m_MeshPool;

    // Queue of chunk coordinates whose meshes need rebuilding.
    std::queue<glm::ivec3> m_MeshRebuildQueue;
    std::unordered_set<glm::ivec3, IVec3Hasher> m_PendingMeshUpdates;

    // Async generation ----------------------------------------------------
    ThreadSafeQueue<glm::ivec3> m_GenerationRequests;
    ThreadSafeQueue<std::unique_ptr<Chunk>> m_GenerationResults;

    std::unordered_set<glm::ivec3, IVec3Hasher> m_PendingGeneration;

    std::atomic<bool> m_StopGeneration{ false };
    std::thread m_GenerationThread;
    bool m_GenerationThreadStarted = false;
};