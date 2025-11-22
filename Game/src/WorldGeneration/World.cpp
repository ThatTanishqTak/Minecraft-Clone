#include "World.h"

#include <algorithm>
#include <cmath>

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

World::World(ChunkMesher* chunkMesher, const Engine::Texture2D* texture) : m_ChunkMesher(chunkMesher), m_Texture(texture)
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

    // Remove chunks that drifted outside the desired radius.
    for (auto it_Chunk = m_ActiveChunks.begin(); it_Chunk != m_ActiveChunks.end();)
    {
        if (l_DesiredChunks.find(it_Chunk->first) == l_DesiredChunks.end())
        {
            GAME_TRACE("Unloading chunk at ({}, {}, {})", it_Chunk->first.x, it_Chunk->first.y, it_Chunk->first.z);
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
    for (auto& it_ChunkEntry : m_ActiveChunks)
    {
        MeshChunkIfDirty(it_ChunkEntry.second);
    }
}

void World::Shutdown()
{
    // Clearing the map releases chunk memory and GPU buffers through destructors.
    m_ActiveChunks.clear();
    GAME_INFO("World shut down and all chunks released");
}

float World::SampleTerrainHeight(int worldX, int worldZ)
{
    const float l_Sample = std::sin(static_cast<float>(worldX) * 0.3f) + std::cos(static_cast<float>(worldZ) * 0.3f);
    const float l_Height = 4.0f + l_Sample * 2.0f;

    return l_Height;
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
    MeshChunkIfDirty(a_EmplaceResult.first->second);

    GAME_TRACE("Chunk created and meshed at ({}, {}, {})", chunkCoordinate.x, chunkCoordinate.y, chunkCoordinate.z);
}

void World::PopulateChunkBlocks(Chunk& chunk) const
{
    // Apply the height sampling formula in world space so neighboring chunks align seamlessly.
    const glm::ivec3 l_WorldOffset = chunk.GetPosition() * Chunk::CHUNK_SIZE;

    for (int l_Z = 0; l_Z < Chunk::CHUNK_SIZE; ++l_Z)
    {
        for (int l_X = 0; l_X < Chunk::CHUNK_SIZE; ++l_X)
        {
            const int l_WorldX = l_WorldOffset.x + l_X;
            const int l_WorldZ = l_WorldOffset.z + l_Z;
            const int l_SurfaceHeight = static_cast<int>(SampleTerrainHeight(l_WorldX, l_WorldZ));

            for (int l_Y = 0; l_Y < Chunk::CHUNK_SIZE; ++l_Y)
            {
                BlockId l_Block = BlockId::Air;

                if (l_Y < l_SurfaceHeight - 2)
                {
                    l_Block = BlockId::Stone;
                }
                else if (l_Y < l_SurfaceHeight - 1)
                {
                    l_Block = BlockId::Dirt;
                }
                else if (l_Y == l_SurfaceHeight - 1)
                {
                    l_Block = BlockId::Grass;
                }

                chunk.SetBlock(l_X, l_Y, l_Z, l_Block);
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

    const MeshedChunk l_MeshedChunk = m_ChunkMesher->Mesh(*chunkData.m_Chunk);
    chunkData.m_Renderer->UpdateMesh(l_MeshedChunk);
    chunkData.m_IsDirty = false;
}