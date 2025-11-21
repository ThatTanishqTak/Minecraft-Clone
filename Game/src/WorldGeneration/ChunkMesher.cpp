#include "ChunkMesher.h"

#include "Engine/Core/Log.h"

#include <array>

ChunkMesher::ChunkMesher(const TextureAtlas* textureAtlas) : m_TextureAtlas(textureAtlas)
{
    // Capture atlas pointer up front so meshing can emit UVs for each quad.
    GAME_TRACE("ChunkMesher constructed with texture atlas: {}", m_TextureAtlas != nullptr);
}

MeshedChunk ChunkMesher::Mesh(const Chunk& chunk) const
{
    GAME_TRACE("Meshing chunk at position ({}, {}, {})", chunk.GetPosition().x, chunk.GetPosition().y, chunk.GetPosition().z);

    MeshedChunk l_Output{};

    // Build meshes for each face direction independently using greedy quads.
    BuildFaceQuads(chunk, BlockFace::East, l_Output);
    BuildFaceQuads(chunk, BlockFace::West, l_Output);
    BuildFaceQuads(chunk, BlockFace::Top, l_Output);
    BuildFaceQuads(chunk, BlockFace::Bottom, l_Output);
    BuildFaceQuads(chunk, BlockFace::North, l_Output);
    BuildFaceQuads(chunk, BlockFace::South, l_Output);

    GAME_TRACE("Meshing complete with {} vertices and {} indices", l_Output.m_Vertices.size(), l_Output.m_Indices.size());

    return l_Output;
}

void ChunkMesher::BuildFaceQuads(const Chunk& chunk, BlockFace face, MeshedChunk& outMesh) const
{
    GAME_TRACE("Building quads for face {}", static_cast<int>(face));

    // Greedy meshing across a plane for the specified face.
    const int l_Size = Chunk::CHUNK_SIZE;

    std::vector<BlockId> l_Mask(static_cast<size_t>(l_Size * l_Size), BlockId::Air);

    for (int w = 0; w < l_Size; ++w)
    {
        // Fill mask for current slice.
        for (int v = 0; v < l_Size; ++v)
        {
            for (int u = 0; u < l_Size; ++u)
            {
                int x = 0;
                int y = 0;
                int z = 0;

                if (face == BlockFace::East || face == BlockFace::West)
                {
                    x = (face == BlockFace::East) ? w : l_Size - 1 - w;
                    y = u;
                    z = v;
                }
                else if (face == BlockFace::Top || face == BlockFace::Bottom)
                {
                    x = u;
                    y = (face == BlockFace::Top) ? l_Size - 1 - w : w;
                    z = v;
                }
                else // North/South
                {
                    x = u;
                    y = v;
                    z = (face == BlockFace::North) ? w : l_Size - 1 - w;
                }

                const BlockId l_Block = chunk.GetBlock(x, y, z);
                const bool l_IsVisible = l_Block != BlockId::Air && chunk.IsFaceVisible(x, y, z, face);
                l_Mask[static_cast<size_t>(v * l_Size + u)] = l_IsVisible ? l_Block : BlockId::Air;
            }
        }

        int v = 0;
        while (v < l_Size)
        {
            int u = 0;
            while (u < l_Size)
            {
                const BlockId l_Current = l_Mask[static_cast<size_t>(v * l_Size + u)];
                if (l_Current == BlockId::Air)
                {
                    ++u;
                    continue;
                }

                int l_Width = 1;
                while (u + l_Width < l_Size && l_Mask[static_cast<size_t>(v * l_Size + u + l_Width)] == l_Current)
                {
                    ++l_Width;
                }

                int l_Height = 1;
                bool l_Done = false;
                while (v + l_Height < l_Size && !l_Done)
                {
                    for (int l_Test = 0; l_Test < l_Width; ++l_Test)
                    {
                        if (l_Mask[static_cast<size_t>((v + l_Height) * l_Size + u + l_Test)] != l_Current)
                        {
                            l_Done = true;
                            break;
                        }
                    }

                    if (!l_Done)
                    {
                        ++l_Height;
                    }
                }

                for (int l_CoverV = 0; l_CoverV < l_Height; ++l_CoverV)
                {
                    for (int l_CoverU = 0; l_CoverU < l_Width; ++l_CoverU)
                    {
                        l_Mask[static_cast<size_t>((v + l_CoverV) * l_Size + u + l_CoverU)] = BlockId::Air;
                    }
                }

                glm::vec3 l_Origin{ 0.0f };
                glm::vec3 l_UDirection{ 0.0f };
                glm::vec3 l_VDirection{ 0.0f };
                glm::vec3 l_Normal{ 0.0f };

                if (face == BlockFace::East)
                {
                    l_Origin = glm::vec3{ static_cast<float>(w + 1), static_cast<float>(u), static_cast<float>(v) };
                    l_UDirection = glm::vec3{ 0.0f, static_cast<float>(l_Width), 0.0f };
                    l_VDirection = glm::vec3{ 0.0f, 0.0f, static_cast<float>(l_Height) };
                    l_Normal = glm::vec3{ 1.0f, 0.0f, 0.0f };
                }
                else if (face == BlockFace::West)
                {
                    l_Origin = glm::vec3{ static_cast<float>(l_Size - w - 1), static_cast<float>(u), static_cast<float>(v + l_Height) };
                    l_UDirection = glm::vec3{ 0.0f, static_cast<float>(l_Width), 0.0f };
                    l_VDirection = glm::vec3{ 0.0f, 0.0f, -static_cast<float>(l_Height) };
                    l_Normal = glm::vec3{ -1.0f, 0.0f, 0.0f };
                }
                else if (face == BlockFace::Top)
                {
                    l_Origin = glm::vec3{ static_cast<float>(u), static_cast<float>(l_Size - w), static_cast<float>(v) };
                    l_UDirection = glm::vec3{ static_cast<float>(l_Width), 0.0f, 0.0f };
                    l_VDirection = glm::vec3{ 0.0f, 0.0f, static_cast<float>(l_Height) };
                    l_Normal = glm::vec3{ 0.0f, 1.0f, 0.0f };
                }
                else if (face == BlockFace::Bottom)
                {
                    l_Origin = glm::vec3{ static_cast<float>(u), static_cast<float>(w), static_cast<float>(v + l_Height) };
                    l_UDirection = glm::vec3{ static_cast<float>(l_Width), 0.0f, 0.0f };
                    l_VDirection = glm::vec3{ 0.0f, 0.0f, -static_cast<float>(l_Height) };
                    l_Normal = glm::vec3{ 0.0f, -1.0f, 0.0f };
                }
                else if (face == BlockFace::North)
                {
                    l_Origin = glm::vec3{ static_cast<float>(u), static_cast<float>(v), static_cast<float>(w + 1) };
                    l_UDirection = glm::vec3{ static_cast<float>(l_Width), 0.0f, 0.0f };
                    l_VDirection = glm::vec3{ 0.0f, static_cast<float>(l_Height), 0.0f };
                    l_Normal = glm::vec3{ 0.0f, 0.0f, 1.0f };
                }
                else // South
                {
                    l_Origin = glm::vec3{ static_cast<float>(u), static_cast<float>(v + l_Height), static_cast<float>(l_Size - w - 1) };
                    l_UDirection = glm::vec3{ static_cast<float>(l_Width), 0.0f, 0.0f };
                    l_VDirection = glm::vec3{ 0.0f, -static_cast<float>(l_Height), 0.0f };
                    l_Normal = glm::vec3{ 0.0f, 0.0f, -1.0f };
                }

                EmitQuad(l_Origin, l_UDirection, l_VDirection, l_Normal, l_Current, face, outMesh);

                u += l_Width;
            }
            ++v;
        }
    }
}

void ChunkMesher::EmitQuad(const glm::vec3& origin, const glm::vec3& uDirection, const glm::vec3& vDirection, const glm::vec3& normal,
    BlockId blockId, BlockFace face, MeshedChunk& outMesh) const
{
    // Generate four vertices forming a quad using supplied orientation vectors.
    const glm::vec3 l_P0 = origin;
    const glm::vec3 l_P1 = origin + vDirection;
    const glm::vec3 l_P2 = origin + uDirection + vDirection;
    const glm::vec3 l_P3 = origin + uDirection;

    const glm::vec3 l_Color = GetBlockFaceColor(blockId, face);

    BlockFaceUV l_FaceUV{};
    if (m_TextureAtlas != nullptr)
    {
        l_FaceUV = m_TextureAtlas->GetFaceUVs(blockId, face);
    }

    // Map UVs to the quad corners in winding order (P0 -> P1 -> P2 -> P3).
    const std::array<glm::vec2, 4> l_UVs = {
        l_FaceUV.m_UV00,
        l_FaceUV.m_UV01,
        l_FaceUV.m_UV11,
        l_FaceUV.m_UV10
    };

    const std::array<Engine::Mesh::Vertex, 4> l_Vertices = {
        Engine::Mesh::Vertex{ l_P0, normal, l_Color, l_UVs[0] },
        Engine::Mesh::Vertex{ l_P1, normal, l_Color, l_UVs[1] },
        Engine::Mesh::Vertex{ l_P2, normal, l_Color, l_UVs[2] },
        Engine::Mesh::Vertex{ l_P3, normal, l_Color, l_UVs[3] },
    };

    const uint32_t l_StartIndex = static_cast<uint32_t>(outMesh.m_Vertices.size());
    outMesh.m_Vertices.insert(outMesh.m_Vertices.end(), l_Vertices.begin(), l_Vertices.end());

    const std::array<uint32_t, 6> l_QuadIndices = {
        l_StartIndex + 0, l_StartIndex + 1, l_StartIndex + 2,
        l_StartIndex + 2, l_StartIndex + 3, l_StartIndex + 0
    };

    outMesh.m_Indices.insert(outMesh.m_Indices.end(), l_QuadIndices.begin(), l_QuadIndices.end());

    GAME_TRACE("Emitted quad at origin ({}, {}, {}) with normal ({}, {}, {})", origin.x, origin.y, origin.z, normal.x, normal.y, normal.z);
}

glm::vec3 ChunkMesher::GetBlockFaceColor(BlockId blockId, BlockFace face) const
{
    // Provide simple, readable colors for each block type to replace UV sampling.
    // Slightly tint the colors per face to give subtle variation and depth cues.
    const std::array<glm::vec3, 4> l_BaseColors = {
        glm::vec3{ 0.0f, 0.0f, 0.0f }, // Air (unused)
        glm::vec3{ 0.35f, 0.70f, 0.25f }, // Grass
        glm::vec3{ 0.55f, 0.35f, 0.20f }, // Dirt
        glm::vec3{ 0.55f, 0.55f, 0.55f }, // Stone
    };

    const glm::vec3 l_BaseColor = l_BaseColors.at(static_cast<size_t>(blockId));

    const std::array<glm::vec3, static_cast<size_t>(BlockFace::Count)> l_FaceTints = {
        glm::vec3{ 1.00f, 1.00f, 1.00f }, // North
        glm::vec3{ 0.95f, 0.95f, 0.95f }, // South
        glm::vec3{ 0.90f, 0.90f, 0.90f }, // East
        glm::vec3{ 0.90f, 0.90f, 0.90f }, // West
        glm::vec3{ 1.05f, 1.05f, 1.05f }, // Top
        glm::vec3{ 0.85f, 0.85f, 0.85f }, // Bottom
    };

    const glm::vec3 l_Tint = l_FaceTints.at(static_cast<size_t>(face));
    return glm::clamp(l_BaseColor * l_Tint, glm::vec3{ 0.0f }, glm::vec3{ 1.0f });
}