#pragma once

#include <array>
#include <glm/glm.hpp>

#include "Block.h"
#include "Chunk.h"

// Configuration settings keep procedural terrain stable by controlling noise parameters and seed.
struct WorldGeneratorConfig
{
    uint32_t m_Seed = 1337u;
    int m_BaseHeight = 8;
    int m_HeightAmplitude = 6;
    float m_HeightFrequency = 0.035f;
    float m_BiomeFrequency = 0.0125f;
    float m_BiomeStrength = 3.0f;
    float m_CaveFrequency = 0.08f;
    float m_CaveThreshold = 0.18f;
    int m_SoilDepth = 3;
    bool m_EnableNoise = true; // Toggle to disable noise for deterministic flat worlds used during threading tests.
};

// GeneratedColumn stores the surface height and block stack for a single (x, z) position within a chunk.
struct GeneratedColumn
{
    int m_SurfaceHeight = 0;
    std::array<BlockId, Chunk::CHUNK_SIZE> m_Blocks{};
};

// WorldGenerator produces reproducible terrain using seeded Perlin-like noise in 2D/3D space.
class WorldGenerator
{
public:
    explicit WorldGenerator(WorldGeneratorConfig config = {});

    const WorldGeneratorConfig& GetConfig() const { return m_Config; }

    // Sample the terrain height in world space using the configured noise parameters.
    int CalculateSurfaceHeight(int worldX, int worldZ) const;

    // Query whether a world-space position should be carved out as a cave.
    bool IsCave(int worldX, int worldY, int worldZ) const;

    // Generate a full vertical column of blocks for the requested chunk-local coordinate.
    // (Still available for any legacy callers; World now uses CalculateSurfaceHeight/IsCave directly.)
    GeneratedColumn GenerateColumn(const glm::ivec3& chunkCoordinate, int localX, int localZ) const;

private:
    float Fade(float value) const;
    float Lerp(float a, float b, float t) const;
    float Gradient(int hash, float x, float y, float z) const;
    float SamplePerlin(float x, float y, float z) const;

private:
    WorldGeneratorConfig m_Config{};
    std::array<int, 512> m_Permutations{};
};