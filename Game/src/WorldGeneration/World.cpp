#include "World.h"

#include <algorithm>
#include <utility>

#include "Engine/Core/Log.h"
#include "Engine/Renderer/Texture2D.h"

namespace
{
    // Combine integer hashes for glm::ivec3 keys.
    size_t HashCombine(size_t seed, size_t value)
    {
        return seed ^ (value + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    }
}

size_t World::IVec3Hasher::operator()(const glm::ivec3& key) const noexcept
{
    size_t l_Hash = std::hash<int>{}(key.x);
    l_Hash = HashCombine(l_Hash, std::hash<int>{}(key.y));
    l_Hash = HashCombine(l_Hash, std::hash<int>{}(key.z));

    return l_Hash;
}

World::World(ChunkMesher* chunkMesher, const Engine::Texture2D* texture, const WorldGenerator* worldGenerator) : m_ChunkMesher(chunkMesher), m_Texture(texture),
    m_WorldGenerator(worldGenerator)
{
    // The world streams chunks near the player to avoid simulating infinity. This constructor wires
    // meshing and texturing dependencies so new chunks can be generated immediately when requested.
    StartWorkerThread();
    GAME_TRACE("World created with render distance {} and worker thread running", m_RenderDistance);
}

World::~World()
{
    Shutdown();
}

void World::SetRenderDistance(int renderDistance)
{
    const int l_ClampedDistance = std::max(1, renderDistance);
    m_RenderDistance = l_ClampedDistance;
    GAME_INFO("World render distance set to {} chunks", m_RenderDistance);
}

void World::UpdateActiveChunks(const glm::ivec3& centerChunkCoordinate)
{
    // Drain completed jobs first so any newly built chunks are visible before we enqueue more work.
    ProcessCompletedChunks();

    // Determine which chunk coordinates should stay loaded around the camera.
    std::unordered_set<glm::ivec3, IVec3Hasher> l_DesiredChunks;
    for (int l_DeltaX = -m_RenderDistance; l_DeltaX <= m_RenderDistance; ++l_DeltaX)
    {
        for (int l_DeltaY = -m_RenderDistance; l_DeltaY <= m_RenderDistance; ++l_DeltaY)
        {
            for (int l_DeltaZ = -m_RenderDistance; l_DeltaZ <= m_RenderDistance; ++l_DeltaZ)
            {
                const glm::ivec3 l_TargetChunk = centerChunkCoordinate + glm::ivec3{ l_DeltaX, l_DeltaY, l_DeltaZ };
                l_DesiredChunks.insert(l_TargetChunk);
            }
        }
    }

    // Remove chunks that drifted outside the desired radius and cancel any pending work for them.
    for (auto it_Chunk = m_ActiveChunks.begin(); it_Chunk != m_ActiveChunks.end();)
    {
        if (l_DesiredChunks.find(it_Chunk->first) == l_DesiredChunks.end())
        {
            GAME_TRACE("Unloading chunk at ({}, {}, {})", it_Chunk->first.x, it_Chunk->first.y, it_Chunk->first.z);
            m_MeshPool.erase(it_Chunk->first);
            m_PendingChunkCoordinates.erase(it_Chunk->first);
            it_Chunk = m_ActiveChunks.erase(it_Chunk);
        }
        else
        {
            ++it_Chunk;
        }
    }

    // Spawn any missing neighbors to keep the render bubble filled, limiting how many jobs start this frame.
    size_t l_JobBudget = m_MaxChunkJobsPerFrame;
    for (const glm::ivec3& it_DesiredChunk : l_DesiredChunks)
    {
        if (l_JobBudget == 0)
        {
            break;
        }

        CreateChunkIfMissing(it_DesiredChunk, l_JobBudget);
    }
}

void World::RefreshChunkMeshes()
{
    // Integrate completed uploads before scheduling fresh work so renderers pick up new meshes immediately.
    ProcessCompletedChunks();

    size_t l_JobBudget = m_MaxChunkJobsPerFrame;
    for (auto& it_ChunkEntry : m_ActiveChunks)
    {
        MeshChunkIfDirty(it_ChunkEntry.second, l_JobBudget, it_ChunkEntry.first);

        if (l_JobBudget == 0)
        {
            break;
        }
    }
}

void World::MarkChunkDirty(const glm::ivec3& chunkCoordinate)
{
    const auto it_Chunk = m_ActiveChunks.find(chunkCoordinate);
    if (it_Chunk == m_ActiveChunks.end())
    {
        return;
    }

    // Flag the chunk so the next refresh pass will regenerate its mesh from cached block data.
    it_Chunk->second.m_IsDirty = true;
}

void World::Shutdown()
{
    // Shutting down the worker prevents background threads from touching engine state during destruction.
    StopWorkerThread();

    // Clearing the map releases chunk memory and GPU buffers through destructors.
    m_ActiveChunks.clear();
    m_MeshPool.clear();
    m_PendingChunkCoordinates.clear();
    GAME_INFO("World shut down and all chunks released");
}

void World::CreateChunkIfMissing(const glm::ivec3& chunkCoordinate, size_t& jobBudget)
{
    if (m_ActiveChunks.find(chunkCoordinate) != m_ActiveChunks.end())
    {
        return;
    }

    ActiveChunk l_NewChunk;
    l_NewChunk.m_Renderer = std::make_unique<ChunkRenderer>();
    l_NewChunk.m_Renderer->SetTexture(m_Texture);
    l_NewChunk.m_IsDirty = true;

    auto a_EmplaceResult = m_ActiveChunks.emplace(chunkCoordinate, std::move(l_NewChunk));
    if (!a_EmplaceResult.second)
    {
        return;
    }

    GAME_TRACE("Chunk placeholder created at ({}, {}, {}) and queued for worker build", chunkCoordinate.x, chunkCoordinate.y, chunkCoordinate.z);
    QueueChunkBuild(chunkCoordinate, nullptr, jobBudget);
}

void World::PopulateChunkBlocks(Chunk& chunk) const
{
    // Populate the chunk using the procedural generator; fall back to air when no generator is provided.
    if (m_WorldGenerator == nullptr)
    {
        GAME_WARN("WorldGenerator missing; chunk at ({}, {}, {}) will remain empty", chunk.GetPosition().x, chunk.GetPosition().y, chunk.GetPosition().z);

        return;
    }

    for (int l_Z = 0; l_Z < Chunk::CHUNK_SIZE; ++l_Z)
    {
        for (int l_X = 0; l_X < Chunk::CHUNK_SIZE; ++l_X)
        {
            const GeneratedColumn a_Column = m_WorldGenerator->GenerateColumn(chunk.GetPosition(), l_X, l_Z);

            for (int l_Y = 0; l_Y < Chunk::CHUNK_SIZE; ++l_Y)
            {
                chunk.SetBlock(l_X, l_Y, l_Z, a_Column.m_Blocks[l_Y]);
            }
        }
    }
}

void World::MeshChunkOnWorker(const ChunkBuildTask& task)
{
    ChunkBuildResult l_Result{};
    l_Result.m_ChunkCoordinate = task.m_ChunkCoordinate;

    // Build or reuse chunk data off the main thread so rendering stays responsive.
    l_Result.m_Chunk = task.m_ExistingChunk != nullptr ? task.m_ExistingChunk : std::make_shared<Chunk>(task.m_ChunkCoordinate);

    if (task.m_ExistingChunk == nullptr)
    {
        PopulateChunkBlocks(*l_Result.m_Chunk);
    }

    l_Result.m_Chunk->RebuildVisibility();

    if (m_ChunkMesher != nullptr)
    {
        l_Result.m_MeshedChunk = m_ChunkMesher->Mesh(*l_Result.m_Chunk);
    }

    m_CompletedChunkQueue.Push(std::move(l_Result));
}

void World::MeshChunkIfDirty(ActiveChunk& chunkData, size_t& jobBudget, const glm::ivec3& chunkCoordinate)
{
    if (!chunkData.m_IsDirty || chunkData.m_Renderer == nullptr)
    {
        return;
    }

    QueueChunkBuild(chunkCoordinate, chunkData.m_Chunk, jobBudget);
}

void World::ProcessCompletedChunks()
{
    ChunkBuildResult l_Result{};
    while (m_CompletedChunkQueue.TryPop(l_Result))
    {
        m_PendingChunkCoordinates.erase(l_Result.m_ChunkCoordinate);

        const auto it_Chunk = m_ActiveChunks.find(l_Result.m_ChunkCoordinate);
        if (it_Chunk == m_ActiveChunks.end())
        {
            continue;
        }

        ActiveChunk& l_ActiveChunk = it_Chunk->second;
        l_ActiveChunk.m_Chunk = l_Result.m_Chunk;

        auto it_MeshBuffer = m_MeshPool.find(l_Result.m_ChunkCoordinate);
        if (it_MeshBuffer == m_MeshPool.end())
        {
            it_MeshBuffer = m_MeshPool.emplace(l_Result.m_ChunkCoordinate, nullptr).first;
        }

        if (l_ActiveChunk.m_Renderer == nullptr)
        {
            l_ActiveChunk.m_Renderer = std::make_unique<ChunkRenderer>();
        }

        l_ActiveChunk.m_Renderer->SetTexture(m_Texture);

        if (!l_Result.m_MeshedChunk.m_Vertices.empty())
        {
            it_MeshBuffer->second = std::make_shared<Engine::Mesh>(l_Result.m_MeshedChunk.m_Vertices, l_Result.m_MeshedChunk.m_Indices);
            l_ActiveChunk.m_Renderer->UpdateMesh(it_MeshBuffer->second);
        }

        l_ActiveChunk.m_IsDirty = false;
    }
}

bool World::QueueChunkBuild(const glm::ivec3& chunkCoordinate, const std::shared_ptr<Chunk>& existingChunk, size_t& jobBudget)
{
    if (jobBudget == 0)
    {
        return false;
    }

    if (m_PendingChunkCoordinates.find(chunkCoordinate) != m_PendingChunkCoordinates.end())
    {
        return false;
    }

    ChunkBuildTask l_Task{};
    l_Task.m_ChunkCoordinate = chunkCoordinate;
    l_Task.m_ExistingChunk = existingChunk;
    m_PendingChunkCoordinates.insert(chunkCoordinate);

    m_ChunkBuildQueue.Push(std::move(l_Task));
    --jobBudget;

    return true;
}

void World::StartWorkerThread()
{
    // Launch the worker thread that will build chunk data and meshes away from the render loop.
    m_ShouldStop = false;
    m_WorkerThread = std::thread([this]()
        {
            while (!m_ShouldStop.load())
            {
                ChunkBuildTask l_Task{};
                if (!m_ChunkBuildQueue.WaitPop(l_Task, m_ShouldStop))
                {
                    break;
                }

                MeshChunkOnWorker(l_Task);
            }
        });
}

void World::StopWorkerThread()
{
    m_ShouldStop = true;
    m_ChunkBuildQueue.NotifyAll();

    if (m_WorkerThread.joinable())
    {
        m_WorkerThread.join();
    }
}