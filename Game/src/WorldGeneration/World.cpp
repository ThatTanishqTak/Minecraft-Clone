#include "World.h"

#include <algorithm>
#include <vector>

#include "Engine/Core/Log.h"
#include "Engine/Renderer/Texture2D.h"
#include "Engine/Renderer/Mesh.h" 

namespace
{
    // Combine integer hashes for glm::ivec3 keys.
    size_t HashCombine(size_t seed, size_t value)
    {
        return seed ^ (value + 0x9e3779b9u + (seed << 6) + (seed >> 2));
    }
}

size_t World::IVec3Hasher::operator()(const glm::ivec3& key) const noexcept
{
    size_t l_Hash = std::hash<int>{}(key.x);
    l_Hash = HashCombine(l_Hash, std::hash<int>{}(key.y));
    l_Hash = HashCombine(l_Hash, std::hash<int>{}(key.z));

    return l_Hash;
}

World::World(ChunkMesher* chunkMesher, const Engine::Texture2D* texture, const WorldGenerator* worldGenerator) : m_ChunkMesher(chunkMesher),
    m_Texture(texture), m_WorldGenerator(worldGenerator)
{
    GAME_TRACE("World created with render distance {}", m_RenderDistance);
    
    StartGenerationWorker();
}

World::~World()
{
    Shutdown();
}

void World::StartGenerationWorker()
{
    if (m_GenerationThreadStarted)
    {
        return;
    }

    if (m_WorldGenerator == nullptr)
    {
        GAME_WARN("World created without WorldGenerator; generation worker not started");
    
        return;
    }

    m_StopGeneration.store(false);

    m_GenerationThread = std::thread([this]()
        {
            while (!m_StopGeneration.load())
            {
                glm::ivec3 l_ChunkCoordinate{};
                if (!m_GenerationRequests.WaitPop(l_ChunkCoordinate, m_StopGeneration))
                {
                    // Either stop flag set or no more work.
                    break;
                }

                if (m_StopGeneration.load())
                {
                    break;
                }

                auto l_Chunk = std::make_unique<Chunk>(l_ChunkCoordinate);
                PopulateChunkBlocks(*l_Chunk);
                l_Chunk->RebuildVisibility();

                m_GenerationResults.Push(std::move(l_Chunk));
            }
        });

    m_GenerationThreadStarted = true;
}

void World::StopGenerationWorker()
{
    if (!m_GenerationThreadStarted)
    {
        return;
    }

    m_StopGeneration.store(true);
    m_GenerationRequests.NotifyAll();

    if (m_GenerationThread.joinable())
    {
        m_GenerationThread.join();
    }

    m_GenerationThreadStarted = false;
}

void World::SetRenderDistance(int renderDistance)
{
    const int l_ClampedDistance = std::max(1, renderDistance);
    m_RenderDistance = l_ClampedDistance; 
    
    GAME_INFO("World render distance set to {} chunks", m_RenderDistance);
}

void World::UpdateActiveChunks(const glm::ivec3& centerChunkCoordinate)
{
    // 2D "disc" around the camera in XZ, keep Y fixed. 
    std::unordered_set<glm::ivec3, IVec3Hasher> l_DesiredChunks;
    for (int l_DeltaX = -m_RenderDistance; l_DeltaX <= m_RenderDistance; ++l_DeltaX)
    {
        for (int l_DeltaZ = -m_RenderDistance; l_DeltaZ <= m_RenderDistance; ++l_DeltaZ)
        {
            const glm::ivec3 l_TargetChunk = centerChunkCoordinate + glm::ivec3{ l_DeltaX, 0, l_DeltaZ };
            l_DesiredChunks.insert(l_TargetChunk);
        }
    }

    // Remove chunks that drifted outside the desired radius.
    for (auto it_Chunk = m_ActiveChunks.begin(); it_Chunk != m_ActiveChunks.end();)
    {
        if (l_DesiredChunks.find(it_Chunk->first) == l_DesiredChunks.end())
        {
            GAME_TRACE("Unloading chunk at ({}, {}, {})", it_Chunk->first.x, it_Chunk->first.y, it_Chunk->first.z);

            m_MeshPool.erase(it_Chunk->first);
            m_PendingMeshUpdates.erase(it_Chunk->first);
            m_PendingGeneration.erase(it_Chunk->first);

            it_Chunk = m_ActiveChunks.erase(it_Chunk);
        }
        else
        {
            ++it_Chunk;
        }
    }

    // Collect missing chunks that should exist but are not active.
    std::vector<glm::ivec3> l_MissingChunks;
    l_MissingChunks.reserve(l_DesiredChunks.size());

    for (const glm::ivec3& l_Desired : l_DesiredChunks)
    {
        if (m_ActiveChunks.find(l_Desired) == m_ActiveChunks.end())
        {
            l_MissingChunks.push_back(l_Desired);
        }
    }

    // Sort missing chunks by distance from the camera chunk so we fill nearest first.
    std::sort(l_MissingChunks.begin(), l_MissingChunks.end(),
        [&centerChunkCoordinate](const glm::ivec3& a, const glm::ivec3& b)
        {
            const glm::ivec3 l_DeltaA = a - centerChunkCoordinate;
            const glm::ivec3 l_DeltaB = b - centerChunkCoordinate;
            const int l_DistASq = l_DeltaA.x * l_DeltaA.x + l_DeltaA.z * l_DeltaA.z;
            const int l_DistBSq = l_DeltaB.x * l_DeltaB.x + l_DeltaB.z * l_DeltaB.z;

            return l_DistASq < l_DistBSq;
        });

    // Hard budget on new chunk generation submissions per frame.
    constexpr int kMaxNewChunksPerFrame = 2;
    int l_SubmittedThisFrame = 0;

    for (const glm::ivec3& l_ChunkCoordinate : l_MissingChunks)
    {
        if (l_SubmittedThisFrame >= kMaxNewChunksPerFrame)
        {
            break;
        }

        // Only enqueue if not already pending.
        if (m_PendingGeneration.insert(l_ChunkCoordinate).second)
        {
            m_GenerationRequests.Push(l_ChunkCoordinate);
            ++l_SubmittedThisFrame;
        }
    }

    // Integrate any finished generation results this frame.
    PumpGenerationResults();
}

void World::PumpGenerationResults()
{
    constexpr int kMaxIntegrationsPerFrame = 4;
    int l_Integrated = 0;

    std::unique_ptr<Chunk> l_Chunk;

    while (l_Integrated < kMaxIntegrationsPerFrame && m_GenerationResults.TryPop(l_Chunk))
    {
        if (!l_Chunk)
        {
            continue;
        }

        const glm::ivec3 l_ChunkPosition = l_Chunk->GetPosition();

        m_PendingGeneration.erase(l_ChunkPosition);

        // If somehow already present, skip integrating this one.
        if (m_ActiveChunks.find(l_ChunkPosition) != m_ActiveChunks.end())
        {
            continue;
        }

        ActiveChunk l_Active{};
        l_Active.m_Chunk = std::move(l_Chunk);
        l_Active.m_Renderer = std::make_unique<ChunkRenderer>();
        l_Active.m_Renderer->SetTexture(m_Texture);
        l_Active.m_IsDirty = true;

        m_ActiveChunks.emplace(l_ChunkPosition, std::move(l_Active));

        if (m_PendingMeshUpdates.insert(l_ChunkPosition).second)
        {
            m_MeshRebuildQueue.push(l_ChunkPosition);
        }

        ++l_Integrated;
    }
}

void World::RefreshChunkMeshes()
{
    // Limit how many chunks we re-mesh per frame so we don't spike the GPU/CPU.
    constexpr int kMaxMeshRebuildsPerFrame = 2;
    int l_ProcessedCount = 0;

    while (l_ProcessedCount < kMaxMeshRebuildsPerFrame && !m_MeshRebuildQueue.empty())
    {
        const glm::ivec3 l_ChunkCoordinate = m_MeshRebuildQueue.front();
        m_MeshRebuildQueue.pop();
        m_PendingMeshUpdates.erase(l_ChunkCoordinate);

        const auto it_Chunk = m_ActiveChunks.find(l_ChunkCoordinate);
        if (it_Chunk != m_ActiveChunks.end())
        {
            MeshChunkIfDirty(it_Chunk->second);
        }

        ++l_ProcessedCount;
    }
}

void World::MarkChunkDirty(const glm::ivec3& chunkCoordinate)
{
    const auto it_Chunk = m_ActiveChunks.find(chunkCoordinate);
    if (it_Chunk == m_ActiveChunks.end())
    {
        return;
    }

    it_Chunk->second.m_IsDirty = true;

    // Enqueue a mesh rebuild if not already queued for this chunk.
    if (m_PendingMeshUpdates.insert(chunkCoordinate).second)
    {
        m_MeshRebuildQueue.push(chunkCoordinate);
    }
}

void World::Shutdown()
{
    // Stop worker first so it stops touching this object.
    StopGenerationWorker();

    while (!m_MeshRebuildQueue.empty())
    {
        m_MeshRebuildQueue.pop();
    }

    m_PendingMeshUpdates.clear();
    m_PendingGeneration.clear();
    m_MeshPool.clear();
    m_ActiveChunks.clear();

    GAME_INFO("World shut down and all chunks released");
}

void World::CreateChunkIfMissing(const glm::ivec3& chunkCoordinate)
{
    // No longer used in the async path; kept for potential future synchronous use.
    if (m_ActiveChunks.find(chunkCoordinate) != m_ActiveChunks.end())
    {
        return;
    }

    ActiveChunk l_NewChunk;
    l_NewChunk.m_Chunk = std::make_unique<Chunk>(chunkCoordinate);
    PopulateChunkBlocks(*l_NewChunk.m_Chunk);
    l_NewChunk.m_Chunk->RebuildVisibility();

    l_NewChunk.m_Renderer = std::make_unique<ChunkRenderer>();
    l_NewChunk.m_Renderer->SetTexture(m_Texture);
    l_NewChunk.m_IsDirty = true;

    m_ActiveChunks.emplace(chunkCoordinate, std::move(l_NewChunk));

    if (m_PendingMeshUpdates.insert(chunkCoordinate).second)
    {
        m_MeshRebuildQueue.push(chunkCoordinate);
    }

    GAME_TRACE("Chunk synchronously created and marked dirty at ({}, {}, {})", chunkCoordinate.x, chunkCoordinate.y, chunkCoordinate.z);
}

void World::PopulateChunkBlocks(Chunk& chunk) const
{
    if (m_WorldGenerator == nullptr)
    {
        GAME_WARN("WorldGenerator missing; chunk at ({}, {}, {}) will remain empty", chunk.GetPosition().x, chunk.GetPosition().y, chunk.GetPosition().z);

        return;
    }

    const glm::ivec3 l_Position = chunk.GetPosition();
    const WorldGeneratorConfig& l_Config = m_WorldGenerator->GetConfig();

    for (int l_LocalZ = 0; l_LocalZ < Chunk::CHUNK_SIZE; ++l_LocalZ)
    {
        for (int l_LocalX = 0; l_LocalX < Chunk::CHUNK_SIZE; ++l_LocalX)
        {
            const int l_GlobalX = l_Position.x * Chunk::CHUNK_SIZE + l_LocalX;
            const int l_GlobalZ = l_Position.z * Chunk::CHUNK_SIZE + l_LocalZ;

            const int l_SurfaceHeight = m_WorldGenerator->CalculateSurfaceHeight(l_GlobalX, l_GlobalZ);

            for (int l_LocalY = 0; l_LocalY < Chunk::CHUNK_SIZE; ++l_LocalY)
            {
                const int l_GlobalY = l_Position.y * Chunk::CHUNK_SIZE + l_LocalY;

                // Default to air.
                BlockId l_Block = BlockId::Air;

                if (l_GlobalY <= l_SurfaceHeight)
                {
                    // Top block: grass.
                    if (l_GlobalY == l_SurfaceHeight)
                    {
                        l_Block = BlockId::Grass;
                    }
                    // Soil layer below: dirt.
                    else if (l_GlobalY >= l_SurfaceHeight - l_Config.m_SoilDepth)
                    {
                        l_Block = BlockId::Dirt;
                    }
                    // Deeper: stone.
                    else
                    {
                        l_Block = BlockId::Stone;
                    }

                    // Carve caves if the generator says so.
                    if (m_WorldGenerator->IsCave(l_GlobalX, l_GlobalY, l_GlobalZ))
                    {
                        l_Block = BlockId::Air;
                    }
                }

                chunk.SetBlock(l_LocalX, l_LocalY, l_LocalZ, l_Block);
            }
        }
    }
}

void World::MeshChunkIfDirty(ActiveChunk& chunkData)
{
    if (!chunkData.m_IsDirty || m_ChunkMesher == nullptr || chunkData.m_Renderer == nullptr)
    {
        return;
    }

    if (chunkData.m_Chunk == nullptr)
    {
        GAME_WARN("MeshChunkIfDirty called with null chunk pointer");
        return;
    }

    const glm::ivec3 l_ChunkPosition = chunkData.m_Chunk->GetPosition();

    const MeshedChunk l_MeshedChunk = m_ChunkMesher->Mesh(*chunkData.m_Chunk);

    auto it_MeshEntry = m_MeshPool.find(l_ChunkPosition);
    if (it_MeshEntry == m_MeshPool.end())
    {
        it_MeshEntry = m_MeshPool.emplace(l_ChunkPosition, nullptr).first;
    }

    it_MeshEntry->second = std::make_shared<Engine::Mesh>(l_MeshedChunk.m_Vertices, l_MeshedChunk.m_Indices);

    chunkData.m_Renderer->SetTexture(m_Texture);
    chunkData.m_Renderer->UpdateMesh(it_MeshEntry->second);

    chunkData.m_IsDirty = false;
}