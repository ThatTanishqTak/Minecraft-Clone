#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace Engine
{
    // Use a lightweight block representation so the mesher stays simple.
    enum class BlockType : std::uint8_t
    {
        Air = 0,
        Solid = 1
    };

    struct Block
    {
        BlockType Type = BlockType::Air;

        bool IsOpaque() const { return Type != BlockType::Air; }
    };

    // Keep a fixed-size chunk that stores block data in a flat array.
    class Chunk
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
        glm::vec2 TexCoord;
    };

    struct ChunkMesh
    {
        std::vector<ChunkVertex> Vertices;
        std::vector<std::uint32_t> Indices;
    };

    // Run a greedy mesher that collapses coplanar faces inside a chunk.
    class ChunkMesher
    {
    public:
        static ChunkMesh GenerateMesh(const Chunk& chunk);
    };
}