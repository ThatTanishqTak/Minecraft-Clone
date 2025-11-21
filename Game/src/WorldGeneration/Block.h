#pragma once

#include <array>
#include <cstdint>
#include <glm/glm.hpp>

// Enumeration of block identifiers present in the sample chunk.
enum class BlockId : uint8_t
{
    Air = 0,
    Grass,
    Dirt,
    Stone,
};

// Faces for a cube block used when calculating visibility and UVs.
enum class BlockFace : uint8_t
{
    North = 0,
    South,
    East,
    West,
    Top,
    Bottom,
    Count
};

// Persistent UV coordinates for a single face of a block.
struct BlockFaceUV
{
    glm::vec2 m_UV00{}; // Lower-left corner.
    glm::vec2 m_UV10{}; // Lower-right corner.
    glm::vec2 m_UV11{}; // Upper-right corner.
    glm::vec2 m_UV01{}; // Upper-left corner.
};