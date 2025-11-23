#include "WorldGenerator.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>

#include "Engine/Core/Log.h"

WorldGenerator::WorldGenerator(WorldGeneratorConfig config) : m_Config(config)
{
    // Build a permutation table seeded from configuration so terrain remains reproducible between sessions.
    std::array<int, 256> l_BasePermutation{};
    std::iota(l_BasePermutation.begin(), l_BasePermutation.end(), 0);

    std::mt19937 l_Random(static_cast<uint32_t>(m_Config.m_Seed));
    std::shuffle(l_BasePermutation.begin(), l_BasePermutation.end(), l_Random);

    for (size_t it_Index = 0; it_Index < l_BasePermutation.size(); ++it_Index)
    {
        m_Permutations[it_Index] = l_BasePermutation[it_Index];
        m_Permutations[it_Index + l_BasePermutation.size()] = l_BasePermutation[it_Index];
    }

    // Previously logged initialization at INFO level; removed to keep logs focused on higher-level systems.
    // GAME_INFO("WorldGenerator initialized with seed {} and base height {}", m_Config.m_Seed, m_Config.m_BaseHeight);
}

int WorldGenerator::CalculateSurfaceHeight(int worldX, int worldZ) const
{
    const float l_ScaledX = static_cast<float>(worldX) * m_Config.m_HeightFrequency;
    const float l_ScaledZ = static_cast<float>(worldZ) * m_Config.m_HeightFrequency;

    // 2D noise establishes the basic elevation.
    const float l_ElevationNoise = SamplePerlin(l_ScaledX, 0.0f, l_ScaledZ);

    // A low-frequency biome noise gently warps hills and valleys to avoid a flat monotone landscape.
    const float l_BiomeNoise = SamplePerlin(
        static_cast<float>(worldX) * m_Config.m_BiomeFrequency,
        0.0f,
        static_cast<float>(worldZ) * m_Config.m_BiomeFrequency
    );
    const float l_BiomeOffset = l_BiomeNoise * m_Config.m_BiomeStrength;

    const float l_Height = static_cast<float>(m_Config.m_BaseHeight) +
        l_BiomeOffset +
        l_ElevationNoise * static_cast<float>(m_Config.m_HeightAmplitude);

    const int l_SurfaceHeight = static_cast<int>(std::round(l_Height));
    return std::max(1, l_SurfaceHeight);
}

GeneratedColumn WorldGenerator::GenerateColumn(const glm::ivec3& chunkCoordinate, int localX, int localZ) const
{
    GeneratedColumn l_Column{};

    const int l_WorldX = chunkCoordinate.x * Chunk::CHUNK_SIZE + localX;
    const int l_WorldZ = chunkCoordinate.z * Chunk::CHUNK_SIZE + localZ;
    l_Column.m_SurfaceHeight = CalculateSurfaceHeight(l_WorldX, l_WorldZ);

    const float l_CaveFrequency = m_Config.m_CaveFrequency;

    for (int l_LocalY = 0; l_LocalY < Chunk::CHUNK_SIZE; ++l_LocalY)
    {
        const int l_WorldY = chunkCoordinate.y * Chunk::CHUNK_SIZE + l_LocalY;
        BlockId l_Block = BlockId::Air;

        if (l_WorldY <= l_Column.m_SurfaceHeight)
        {
            // Use 3D noise to carve caves beneath the surface while preserving topsoil and grass.
            const float l_CaveNoise = std::abs(SamplePerlin(
                static_cast<float>(l_WorldX) * l_CaveFrequency,
                static_cast<float>(l_WorldY) * l_CaveFrequency,
                static_cast<float>(l_WorldZ) * l_CaveFrequency
            ));
            const bool l_IsOpenCave =
                l_CaveNoise < m_Config.m_CaveThreshold && l_WorldY < l_Column.m_SurfaceHeight - 1;

            if (!l_IsOpenCave)
            {
                if (l_WorldY < l_Column.m_SurfaceHeight - m_Config.m_SoilDepth)
                {
                    l_Block = BlockId::Stone;
                }
                else if (l_WorldY < l_Column.m_SurfaceHeight)
                {
                    l_Block = BlockId::Dirt;
                }
                else
                {
                    l_Block = BlockId::Grass;
                }
            }
        }

        l_Column.m_Blocks[l_LocalY] = l_Block;
    }

    return l_Column;
}

float WorldGenerator::Fade(float value) const
{
    return value * value * value * (value * (value * 6.0f - 15.0f) + 10.0f);
}

float WorldGenerator::Lerp(float a, float b, float t) const
{
    return a + t * (b - a);
}

float WorldGenerator::Gradient(int hash, float x, float y, float z) const
{
    const int l_H = hash & 15;
    const float l_U = l_H < 8 ? x : y;
    const float l_V = l_H < 4 ? y : (l_H == 12 || l_H == 14 ? x : z);

    return ((l_H & 1) == 0 ? l_U : -l_U) + ((l_H & 2) == 0 ? l_V : -l_V);
}

float WorldGenerator::SamplePerlin(float x, float y, float z) const
{
    const int l_X = static_cast<int>(std::floor(x)) & 255;
    const int l_Y = static_cast<int>(std::floor(y)) & 255;
    const int l_Z = static_cast<int>(std::floor(z)) & 255;

    const float l_XRelative = x - std::floor(x);
    const float l_YRelative = y - std::floor(y);
    const float l_ZRelative = z - std::floor(z);

    const float l_U = Fade(l_XRelative);
    const float l_V = Fade(l_YRelative);
    const float l_W = Fade(l_ZRelative);

    const int l_A = m_Permutations[l_X] + l_Y;
    const int l_AA = m_Permutations[l_A] + l_Z;
    const int l_AB = m_Permutations[l_A + 1] + l_Z;
    const int l_B = m_Permutations[l_X + 1] + l_Y;
    const int l_BA = m_Permutations[l_B] + l_Z;
    const int l_BB = m_Permutations[l_B + 1] + l_Z;

    const float l_GradientAA = Gradient(m_Permutations[l_AA], l_XRelative, l_YRelative, l_ZRelative);
    const float l_GradientBA = Gradient(m_Permutations[l_BA], l_XRelative - 1, l_YRelative, l_ZRelative);
    const float l_GradientAB = Gradient(m_Permutations[l_AB], l_XRelative, l_YRelative - 1, l_ZRelative);
    const float l_GradientBB = Gradient(m_Permutations[l_BB], l_XRelative - 1, l_YRelative - 1, l_ZRelative);

    const float l_GradientAA1 = Gradient(m_Permutations[l_AA + 1], l_XRelative, l_YRelative, l_ZRelative - 1);
    const float l_GradientBA1 = Gradient(m_Permutations[l_BA + 1], l_XRelative - 1, l_YRelative, l_ZRelative - 1);
    const float l_GradientAB1 = Gradient(m_Permutations[l_AB + 1], l_XRelative, l_YRelative - 1, l_ZRelative - 1);
    const float l_GradientBB1 = Gradient(m_Permutations[l_BB + 1], l_XRelative - 1, l_YRelative - 1, l_ZRelative - 1);

    const float l_XLerp0 = Lerp(l_GradientAA, l_GradientBA, l_U);
    const float l_XLerp1 = Lerp(l_GradientAB, l_GradientBB, l_U);
    const float l_XLerp2 = Lerp(l_GradientAA1, l_GradientBA1, l_U);
    const float l_XLerp3 = Lerp(l_GradientAB1, l_GradientBB1, l_U);

    const float l_YLerp0 = Lerp(l_XLerp0, l_XLerp1, l_V);
    const float l_YLerp1 = Lerp(l_XLerp2, l_XLerp3, l_V);
    const float l_Result = Lerp(l_YLerp0, l_YLerp1, l_W);

    // Normalize the result to [-1, 1] since the gradient sums can exceed unit length.
    return std::clamp(l_Result, -1.0f, 1.0f);
}