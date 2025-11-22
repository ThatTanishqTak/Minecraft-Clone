#pragma once

#include <array>
#include <vector>
#include <glm/glm.hpp>

#include "Block.h"

// Represents a fixed-size block chunk storing identifiers and per-face visibility flags.
class Chunk
{
public:
    static constexpr int CHUNK_SIZE = 32;
    static constexpr int CHUNK_VOLUME = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

public:
    explicit Chunk(const glm::ivec3& position = glm::ivec3{ 0 });

    void SetBlock(int x, int y, int z, BlockId blockID);
    BlockId GetBlock(int x, int y, int z) const;

    // Precompute which faces are visible so meshing can quickly skip occluded quads.
    void RebuildVisibility();
    bool IsFaceVisible(int x, int y, int z, BlockFace face) const;

    const glm::ivec3& GetPosition() const { return m_Position; }

private:
    size_t ToIndex(int x, int y, int z) const;
    bool IsInsideChunk(int x, int y, int z) const;

private:
    glm::ivec3 m_Position{ 0 };
    std::array<BlockId, CHUNK_VOLUME> m_BlockIds{};
    std::array<uint8_t, CHUNK_VOLUME> m_VisibilityMasks{}; // Bitmask matching BlockFace order.
};