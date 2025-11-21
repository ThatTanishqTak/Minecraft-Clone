#include "Chunk.h"

#include <algorithm>

Chunk::Chunk(const glm::ivec3& position) : m_Position(position)
{
    // Default to air blocks to keep visibility calculations simple until blocks are placed.
    m_BlockIds.fill(BlockId::Air);
    m_VisibilityMasks.fill(0);
}

void Chunk::SetBlock(int x, int y, int z, BlockId blockId)
{
    const size_t l_Index = ToIndex(x, y, z);
    m_BlockIds[l_Index] = blockId;
}

BlockId Chunk::GetBlock(int x, int y, int z) const
{
    const size_t l_Index = ToIndex(x, y, z);
    return m_BlockIds[l_Index];
}

void Chunk::RebuildVisibility()
{
    // Determine which faces are exposed by checking for air/out-of-bounds neighbors.
    for (int l_Z = 0; l_Z < CHUNK_SIZE; ++l_Z)
    {
        for (int l_Y = 0; l_Y < CHUNK_SIZE; ++l_Y)
        {
            for (int l_X = 0; l_X < CHUNK_SIZE; ++l_X)
            {
                const size_t l_Index = ToIndex(l_X, l_Y, l_Z);
                const BlockId l_Block = m_BlockIds[l_Index];
                if (l_Block == BlockId::Air)
                {
                    m_VisibilityMasks[l_Index] = 0;
                    continue;
                }

                uint8_t l_Mask = 0;

                auto l_CheckFace = [&](BlockFace l_Face, int l_OffsetX, int l_OffsetY, int l_OffsetZ)
                    {
                        const int l_NeighborX = l_X + l_OffsetX;
                        const int l_NeighborY = l_Y + l_OffsetY;
                        const int l_NeighborZ = l_Z + l_OffsetZ;

                        const bool l_HasNeighbor = IsInsideChunk(l_NeighborX, l_NeighborY, l_NeighborZ);
                        const BlockId l_NeighborBlock = l_HasNeighbor ? GetBlock(l_NeighborX, l_NeighborY, l_NeighborZ) : BlockId::Air;

                        if (l_NeighborBlock == BlockId::Air)
                        {
                            l_Mask |= (1u << static_cast<uint8_t>(l_Face));
                        }
                    };

                l_CheckFace(BlockFace::East, 1, 0, 0);
                l_CheckFace(BlockFace::West, -1, 0, 0);
                l_CheckFace(BlockFace::Top, 0, 1, 0);
                l_CheckFace(BlockFace::Bottom, 0, -1, 0);
                l_CheckFace(BlockFace::North, 0, 0, 1);
                l_CheckFace(BlockFace::South, 0, 0, -1);

                m_VisibilityMasks[l_Index] = l_Mask;
            }
        }
    }
}

bool Chunk::IsFaceVisible(int x, int y, int z, BlockFace face) const
{
    const size_t l_Index = ToIndex(x, y, z);
    const uint8_t l_Mask = m_VisibilityMasks[l_Index];
    return (l_Mask & (1u << static_cast<uint8_t>(face))) != 0;
}

size_t Chunk::ToIndex(int x, int y, int z) const
{
    const int l_ClampedX = std::clamp(x, 0, CHUNK_SIZE - 1);
    const int l_ClampedY = std::clamp(y, 0, CHUNK_SIZE - 1);
    const int l_ClampedZ = std::clamp(z, 0, CHUNK_SIZE - 1);
    return static_cast<size_t>(l_ClampedX + CHUNK_SIZE * (l_ClampedY + CHUNK_SIZE * l_ClampedZ));
}

bool Chunk::IsInsideChunk(int x, int y, int z) const
{
    return x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE;
}