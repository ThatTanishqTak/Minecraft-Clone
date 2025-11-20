#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include <glm/vec3.hpp>

#include "Engine/Core/Core.h"

namespace Engine
{
    // Use a lightweight block representation so the mesher stays simple.
    enum class BlockType : std::uint8_t
    {
        Air = 0,
        Dirt = 1,
        Grass = 2,
        Stone = 3
    };

    enum class BlockFace : std::uint8_t
    {
        PositiveX = 0,
        NegativeX = 1,
        PositiveY = 2,
        NegativeY = 3,
        PositiveZ = 4,
        NegativeZ = 5
    };

    struct Block
    {
        BlockType Type = BlockType::Air;
    };

    struct BlockRenderInfo
    {
        bool IsOpaque = false;
        float MaterialFlags = 0.0f;
        glm::vec3 Color = glm::vec3(1.0f);
    };

    // Keep a fixed-size chunk that stores block data in a flat array.
    class ENGINE_API Chunk
    {
    public:
        static constexpr int s_SizeX = 16;
        static constexpr int s_SizeY = 16;
        static constexpr int s_SizeZ = 16;

        Chunk();

        Block GetBlock(int x, int y, int z) const;
        void SetBlock(int x, int y, int z, BlockType type);

    private:
        std::size_t GetIndex(int x, int y, int z) const;

    private:
        std::array<Block, s_SizeX* s_SizeY* s_SizeZ> m_Blocks;
    };

    // Emit this vertex format from the mesher so the renderer has position, normal, and UVs.
    struct ChunkVertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Color;
        float MaterialFlags = 0.0f;
    };

    struct ChunkMesh
    {
        std::vector<ChunkVertex> Vertices;
        std::vector<std::uint32_t> Indices;
    };

    // Run a greedy mesher that collapses coplanar faces inside a chunk.
    class ENGINE_API ChunkMesher
    {
    public:
        static ChunkMesh GenerateMesh(const Chunk& chunk, const std::unordered_map<BlockType, BlockRenderInfo>& blockTable);
    };
}