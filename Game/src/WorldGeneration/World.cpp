#include "World.h"

#include <algorithm>

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

World::World(ChunkMesher* chunkMesher, const Engine::Texture2D* texture, const WorldGenerator* worldGenerator)
    : m_ChunkMesher(chunkMesher)
    , m_Texture(texture)
    , m_WorldGenerator(worldGenerator)
{
    // The world streams chunks near the player to avoid simulating infinity. This constructor wires
    // meshing and texturing dependencies so new chunks can be generated immediately when requested.
    GAME_TRACE("World created with render distance {}", m_RenderDistance);
}

void World::SetRenderDistance(int renderDistance)
{
    const int l_ClampedDistance = std::max(1, renderDistance);
    m_RenderDistance = l_ClampedDistance;
    GAME_INFO("World render distance set to {} chunks", m_RenderDistance);
}

void World::UpdateActiveChunks(const glm::ivec3& centerChunkCoordinate)
{
    // Determine which chunk coordinates should stay loaded around the camera.
    std::unordered_set<glm::ivec3, IVec3Hasher> l_DesiredChunks;

    // Restrict streaming to a horizontal (X/Z) radius. Vertical chunk range can be expanded later
    // if needed, but staying flat avoids loading unnecessary layers above and below.
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

            // Remove associated mesh and pending mesh updates.
            m_MeshPool.erase(it_Chunk->first);
            m_PendingMeshUpdates.erase(it_Chunk->first);

            it_Chunk = m_ActiveChunks.erase(it_Chunk);
        }
        else
        {
            ++it_Chunk;
        }
    }

    // Spawn any missing neighbors to keep the render bubble filled.
    for (const glm::ivec3& it_DesiredChunk : l_DesiredChunks)
    {
        CreateChunkIfMissing(it_DesiredChunk);
    }
}

void World::RefreshChunkMeshes()
{
    // Process a limited number of mesh rebuild jobs per frame so chunk streaming does not stall
    // the main thread when the camera crosses chunk boundaries.
    constexpr int kMaxMeshRebuildsPerFrame = 2;
    int l_ProcessedCount = 0;

    while (l_ProcessedCount < kMaxMeshRebuildsPerFrame && !m_MeshRebuildQueue.empty())
    {
        const glm::ivec3 l_ChunkCoordinate = m_MeshRebuildQueue.front();
        m_MeshRebuildQueue.pop();
        m_PendingMeshUpdates.erase(l_ChunkCoordinate);

        auto it_Chunk = m_ActiveChunks.find(l_ChunkCoordinate);
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

    // Flag the chunk so the next refresh pass will regenerate its mesh from cached block data.
    it_Chunk->second.m_IsDirty = true;

    // Enqueue a mesh rebuild if this chunk is not already queued.
    if (m_PendingMeshUpdates.insert(chunkCoordinate).second)
    {
        m_MeshRebuildQueue.push(chunkCoordinate);
    }
}

void World::Shutdown()
{
    // Clearing the map releases chunk memory and GPU buffers through destructors.
    m_ActiveChunks.clear();

    // Also clear mesh pool and any pending mesh rebuilds.
    while (!m_MeshRebuildQueue.empty())
    {
        m_MeshRebuildQueue.pop();
    }
    m_PendingMeshUpdates.clear();
    m_MeshPool.clear();

    GAME_INFO("World shut down and all chunks released");
}

void World::CreateChunkIfMissing(const glm::ivec3& chunkCoordinate)
{
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

    auto a_EmplaceResult = m_ActiveChunks.emplace(chunkCoordinate, std::move(l_NewChunk));
    (void)a_EmplaceResult;

    // Queue an initial mesh build for this new chunk so it will appear gradually instead of
    // blocking a single frame with many mesh builds.
    if (m_PendingMeshUpdates.insert(chunkCoordinate).second)
    {
        m_MeshRebuildQueue.push(chunkCoordinate);
    }

    GAME_TRACE("Chunk created and marked dirty at ({}, {}, {})",
        chunkCoordinate.x, chunkCoordinate.y, chunkCoordinate.z);
}

void World::PopulateChunkBlocks(Chunk& chunk) const
{
    // Populate the chunk using the procedural generator; fall back to air when no generator is provided.
    if (m_WorldGenerator == nullptr)
    {
        GAME_WARN("WorldGenerator missing; chunk at ({}, {}, {}) will remain empty",
            chunk.GetPosition().x, chunk.GetPosition().y, chunk.GetPosition().z);

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

void World::MeshChunkIfDirty(ActiveChunk& chunkData)
{
    if (m_ChunkMesher == nullptr || !chunkData.m_IsDirty || chunkData.m_Renderer == nullptr)
    {
        return;
    }

    if (chunkData.m_Chunk == nullptr)
    {
        return;
    }

    const glm::ivec3 l_ChunkCoordinate = chunkData.m_Chunk->GetPosition();

    // Meshing is cached per chunk coordinate so re-uploads reuse pooled buffers when possible.
    const MeshedChunk l_MeshedChunk = m_ChunkMesher->Mesh(*chunkData.m_Chunk);

    auto it_MeshBuffer = m_MeshPool.find(l_ChunkCoordinate);
    if (it_MeshBuffer == m_MeshPool.end())
    {
        it_MeshBuffer = m_MeshPool.emplace(l_ChunkCoordinate, nullptr).first;
    }

    it_MeshBuffer->second = std::make_shared<Engine::Mesh>(l_MeshedChunk.m_Vertices, l_MeshedChunk.m_Indices);
    chunkData.m_Renderer->SetTexture(m_Texture);
    chunkData.m_Renderer->UpdateMesh(it_MeshBuffer->second);
    chunkData.m_IsDirty = false;
}