#include "Engine/World/ChunkMesher.h"

#include <algorithm>

#include <glm/geometric.hpp>

namespace Engine
{
    namespace
    {
        constexpr glm::vec3 s_Axes[3] =
        {
            glm::vec3(1.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.0f, 0.0f, 1.0f)
        };

        struct MaskCell
        {
            bool HasFace = false;
            bool IsPositive = true;
            BlockType Type = BlockType::Air;
        };

        bool IsInsideChunk(int x, int y, int z)
        {
            return x >= 0 && x < Chunk::s_SizeX && y >= 0 && y < Chunk::s_SizeY && z >= 0 && z < Chunk::s_SizeZ;
        }

        MaskCell EvaluateFace(const Chunk& chunk, const std::array<int, 3>& position, const std::array<int, 3>& step)
        {
            MaskCell l_Cell = {};
            Block l_FirstBlock = {};
            Block l_SecondBlock = {};

            if (IsInsideChunk(position[0], position[1], position[2]))
            {
                l_FirstBlock = chunk.GetBlock(position[0], position[1], position[2]);
            }

            std::array<int, 3> l_SecondPosition =
            {
                position[0] + step[0],
                position[1] + step[1],
                position[2] + step[2]
            };

            if (IsInsideChunk(l_SecondPosition[0], l_SecondPosition[1], l_SecondPosition[2]))
            {
                l_SecondBlock = chunk.GetBlock(l_SecondPosition[0], l_SecondPosition[1], l_SecondPosition[2]);
            }

            bool l_FirstOpaque = l_FirstBlock.IsOpaque();
            bool l_SecondOpaque = l_SecondBlock.IsOpaque();

            if (l_FirstOpaque == l_SecondOpaque)
            {
                return l_Cell;
            }

            l_Cell.HasFace = true;
            l_Cell.IsPositive = l_FirstOpaque && !l_SecondOpaque;
            l_Cell.Type = l_FirstOpaque ? l_FirstBlock.Type : l_SecondBlock.Type;

            return l_Cell;
        }

        void AppendQuad(ChunkMesh& mesh, const glm::vec3& origin, const glm::vec3& deltaU, const glm::vec3& deltaV, const glm::vec3& normal, bool isPositive)
        {
            std::uint32_t l_BaseIndex = static_cast<std::uint32_t>(mesh.Vertices.size());

            // Build the quad vertices in an order that keeps the normal consistent.
            if (isPositive)
            {
                float l_DeltaULength = glm::length(deltaU);
                float l_DeltaVLength = glm::length(deltaV);

                mesh.Vertices.push_back({ origin, normal, glm::vec2(0.0f, 0.0f) });
                mesh.Vertices.push_back({ origin + deltaU, normal, glm::vec2(l_DeltaULength, 0.0f) });
                mesh.Vertices.push_back({ origin + deltaU + deltaV, normal, glm::vec2(l_DeltaULength, l_DeltaVLength) });
                mesh.Vertices.push_back({ origin + deltaV, normal, glm::vec2(0.0f, l_DeltaVLength) });
            }
            else
            {
                float l_DeltaULength = glm::length(deltaU);
                float l_DeltaVLength = glm::length(deltaV);

                mesh.Vertices.push_back({ origin, normal, glm::vec2(0.0f, 0.0f) });
                mesh.Vertices.push_back({ origin + deltaV, normal, glm::vec2(0.0f, l_DeltaVLength) });
                mesh.Vertices.push_back({ origin + deltaU + deltaV, normal, glm::vec2(l_DeltaULength, l_DeltaVLength) });
                mesh.Vertices.push_back({ origin + deltaU, normal, glm::vec2(l_DeltaULength, 0.0f) });
            }

            mesh.Indices.push_back(l_BaseIndex + 0);
            mesh.Indices.push_back(l_BaseIndex + 1);
            mesh.Indices.push_back(l_BaseIndex + 2);
            mesh.Indices.push_back(l_BaseIndex + 2);
            mesh.Indices.push_back(l_BaseIndex + 3);
            mesh.Indices.push_back(l_BaseIndex + 0);
        }
    }

    Chunk::Chunk()
    {
        // Initialize every block to air so the chunk starts empty.
        m_Blocks.fill(Block{ BlockType::Air });
    }

    Block Chunk::GetBlock(int x, int y, int z) const
    {
        if (!IsInsideChunk(x, y, z))
        {
            return Block{ BlockType::Air };
        }

        return m_Blocks[GetIndex(x, y, z)];
    }

    void Chunk::SetBlock(int x, int y, int z, BlockType type)
    {
        if (!IsInsideChunk(x, y, z))
        {
            return;
        }

        m_Blocks[GetIndex(x, y, z)] = Block{ type };
    }

    std::size_t Chunk::GetIndex(int x, int y, int z) const
    {
        return static_cast<std::size_t>(x + y * s_SizeX + z * s_SizeX * s_SizeY);
    }

    ChunkMesh ChunkMesher::GenerateMesh(const Chunk& chunk)
    {
        ChunkMesh l_Mesh = {};
        std::array<int, 3> l_Dimensions = { Chunk::s_SizeX, Chunk::s_SizeY, Chunk::s_SizeZ };
        std::array<int, 3> l_Position = { 0, 0, 0 };
        std::array<int, 3> l_Step = { 0, 0, 0 };

        // Iterate over each axis to cull internal faces and merge visible quads.
        for (int l_Axis = 0; l_Axis < 3; ++l_Axis)
        {
            int u = (l_Axis + 1) % 3;
            int v = (l_Axis + 2) % 3;
            l_Step = { 0, 0, 0 };
            l_Step[l_Axis] = 1;

            std::vector<MaskCell> l_Mask(static_cast<std::size_t>(l_Dimensions[u] * l_Dimensions[v]));

            for (l_Position[l_Axis] = 0; l_Position[l_Axis] <= l_Dimensions[l_Axis]; ++l_Position[l_Axis])
            {
                // I fill a 2D mask describing which faces exist on this slice.
                for (l_Position[v] = 0; l_Position[v] < l_Dimensions[v]; ++l_Position[v])
                {
                    for (l_Position[u] = 0; l_Position[u] < l_Dimensions[u]; ++l_Position[u])
                    {
                        std::size_t l_MaskIndex = static_cast<std::size_t>(l_Position[u] + l_Position[v] * l_Dimensions[u]);
                        l_Mask[l_MaskIndex] = EvaluateFace(chunk, l_Position, l_Step);
                    }
                }

                // I apply the greedy meshing algorithm on the mask.
                for (int i = 0; i < l_Dimensions[v]; ++i)
                {
                    int j = 0;
                    while (j < l_Dimensions[u])
                    {
                        std::size_t l_MaskIndex = static_cast<std::size_t>(j + i * l_Dimensions[u]);
                        MaskCell l_Cell = l_Mask[l_MaskIndex];
                        if (!l_Cell.HasFace)
                        {
                            ++j;
                            continue;
                        }

                        int l_Width = 1;
                        while (j + l_Width < l_Dimensions[u])
                        {
                            std::size_t l_NextIndex = static_cast<std::size_t>(j + l_Width + i * l_Dimensions[u]);
                            if (!l_Mask[l_NextIndex].HasFace || l_Mask[l_NextIndex].IsPositive != l_Cell.IsPositive || l_Mask[l_NextIndex].Type != l_Cell.Type)
                            {
                                break;
                            }

                            ++l_Width;
                        }

                        int l_Height = 1;
                        while (i + l_Height < l_Dimensions[v])
                        {
                            bool l_RowMatches = true;
                            for (int k = 0; k < l_Width; ++k)
                            {
                                std::size_t l_RowIndex = static_cast<std::size_t>(j + k + (i + l_Height) * l_Dimensions[u]);
                                if (!l_Mask[l_RowIndex].HasFace || l_Mask[l_RowIndex].IsPositive != l_Cell.IsPositive || l_Mask[l_RowIndex].Type != l_Cell.Type)
                                {
                                    l_RowMatches = false;
                                    break;
                                }
                            }

                            if (!l_RowMatches)
                            {
                                break;
                            }

                            ++l_Height;
                        }

                        // Place the quad on the correct plane and assign the proper winding.
                        std::array<float, 3> l_FaceOrigin =
                        {
                            static_cast<float>(l_Position[0]),
                            static_cast<float>(l_Position[1]),
                            static_cast<float>(l_Position[2])
                        };

                        l_FaceOrigin[u] = static_cast<float>(j);
                        l_FaceOrigin[v] = static_cast<float>(i);
                        l_FaceOrigin[l_Axis] = static_cast<float>(l_Position[l_Axis]) + (l_Cell.IsPositive ? 1.0f : 0.0f);

                        glm::vec3 l_Origin(l_FaceOrigin[0], l_FaceOrigin[1], l_FaceOrigin[2]);
                        glm::vec3 l_DeltaU = s_Axes[u] * static_cast<float>(l_Width);
                        glm::vec3 l_DeltaV = s_Axes[v] * static_cast<float>(l_Height);
                        glm::vec3 l_Normal = s_Axes[l_Axis] * (l_Cell.IsPositive ? 1.0f : -1.0f);

                        AppendQuad(l_Mesh, l_Origin, l_DeltaU, l_DeltaV, l_Normal, l_Cell.IsPositive);

                        for (int l_CoveredY = 0; l_CoveredY < l_Height; ++l_CoveredY)
                        {
                            for (int l_CoveredX = 0; l_CoveredX < l_Width; ++l_CoveredX)
                            {
                                std::size_t l_ClearIndex = static_cast<std::size_t>(j + l_CoveredX + (i + l_CoveredY) * l_Dimensions[u]);
                                l_Mask[l_ClearIndex].HasFace = false;
                            }
                        }

                        j += l_Width;
                    }
                }
            }
        }

        return l_Mesh;
    }
}