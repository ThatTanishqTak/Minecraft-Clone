#pragma once

#include <atomic>
#include <memory>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <glm/glm.hpp>

#include "Chunk.h"
#include "ChunkMesher.h"
#include "ChunkRenderer.h"
#include "ThreadSafeQueue.h"
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
        std::shared_ptr<Chunk> m_Chunk;
        std::unique_ptr<ChunkRenderer> m_Renderer;
        bool m_IsDirty = false;
    };

    struct IVec3Hasher
    {
        size_t operator()(const glm::ivec3& key) const noexcept;
    };

    struct ChunkBuildTask
    {
        glm::ivec3 m_ChunkCoordinate{};
        std::shared_ptr<Chunk> m_ExistingChunk;
    };

    struct ChunkBuildResult
    {
        glm::ivec3 m_ChunkCoordinate{};
        std::shared_ptr<Chunk> m_Chunk;
        MeshedChunk m_MeshedChunk;
    };

public:
    World(ChunkMesher* chunkMesher, const Engine::Texture2D* texture, const WorldGenerator* worldGenerator);
    ~World();

    void SetRenderDistance(int renderDistance);
    void UpdateActiveChunks(const glm::ivec3& centerChunkCoordinate);
    void RefreshChunkMeshes();
    void MarkChunkDirty(const glm::ivec3& chunkCoordinate);

    const std::unordered_map<glm::ivec3, ActiveChunk, IVec3Hasher>& GetActiveChunks() const { return m_ActiveChunks; }

    void Shutdown();

private:
    void CreateChunkIfMissing(const glm::ivec3& chunkCoordinate, size_t& jobBudget);
    void PopulateChunkBlocks(Chunk& chunk) const;
    void MeshChunkOnWorker(const ChunkBuildTask& task);
    void MeshChunkIfDirty(ActiveChunk& chunkData, size_t& jobBudget, const glm::ivec3& chunkCoordinate);
    void ProcessCompletedChunks();
    bool QueueChunkBuild(const glm::ivec3& chunkCoordinate, const std::shared_ptr<Chunk>& existingChunk, size_t& jobBudget);
    void StartWorkerThread();
    void StopWorkerThread();

private:
    ChunkMesher* m_ChunkMesher = nullptr;
    const Engine::Texture2D* m_Texture = nullptr;
    const WorldGenerator* m_WorldGenerator = nullptr;
    int m_RenderDistance = 2;
    std::unordered_map<glm::ivec3, ActiveChunk, IVec3Hasher> m_ActiveChunks;
    std::unordered_map<glm::ivec3, std::shared_ptr<Engine::Mesh>, IVec3Hasher> m_MeshPool;
    ThreadSafeQueue<ChunkBuildTask> m_ChunkBuildQueue;
    ThreadSafeQueue<ChunkBuildResult> m_CompletedChunkQueue;
    std::unordered_set<glm::ivec3, IVec3Hasher> m_PendingChunkCoordinates;
    std::thread m_WorkerThread;
    std::atomic<bool> m_ShouldStop{ false };
    size_t m_MaxChunkJobsPerFrame = 2;
};