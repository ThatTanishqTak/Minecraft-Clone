#include "ChunkMesher.h"

#include <array>

ChunkMesher::ChunkMesher(const TextureAtlas& atlas) : m_Atlas(atlas)
{

}

MeshedChunk ChunkMesher::Mesh(const Chunk& chunk) const
{
    MeshedChunk l_Output{};

    // Build meshes for each face direction independently using greedy quads.
    BuildFaceQuads(chunk, BlockFace::East, l_Output);
    BuildFaceQuads(chunk, BlockFace::West, l_Output);
    BuildFaceQuads(chunk, BlockFace::Top, l_Output);
    BuildFaceQuads(chunk, BlockFace::Bottom, l_Output);
    BuildFaceQuads(chunk, BlockFace::North, l_Output);
    BuildFaceQuads(chunk, BlockFace::South, l_Output);

    return l_Output;
}

void ChunkMesher::BuildFaceQuads(const Chunk& chunk, BlockFace face, MeshedChunk& outMesh) const
{
    // Greedy meshing across a plane for the specified face.
    const int l_Size = Chunk::CHUNK_SIZE;

    std::vector<BlockId> l_Mask(static_cast<size_t>(l_Size * l_Size), BlockId::Air);

    for (int l_W = 0; l_W < l_Size; ++l_W)
    {
        // Fill mask for current slice.
        for (int l_V = 0; l_V < l_Size; ++l_V)
        {
            for (int l_U = 0; l_U < l_Size; ++l_U)
            {
                int l_X = 0;
                int l_Y = 0;
                int l_Z = 0;

                if (face == BlockFace::East || face == BlockFace::West)
                {
                    l_X = (face == BlockFace::East) ? l_W : l_Size - 1 - l_W;
                    l_Y = l_U;
                    l_Z = l_V;
                }
                else if (face == BlockFace::Top || face == BlockFace::Bottom)
                {
                    l_X = l_U;
                    l_Y = (face == BlockFace::Top) ? l_Size - 1 - l_W : l_W;
                    l_Z = l_V;
                }
                else // North/South
                {
                    l_X = l_U;
                    l_Y = l_V;
                    l_Z = (face == BlockFace::North) ? l_W : l_Size - 1 - l_W;
                }

                const BlockId l_Block = chunk.GetBlock(l_X, l_Y, l_Z);
                const bool l_IsVisible = l_Block != BlockId::Air && chunk.IsFaceVisible(l_X, l_Y, l_Z, face);
                l_Mask[static_cast<size_t>(l_V * l_Size + l_U)] = l_IsVisible ? l_Block : BlockId::Air;
            }
        }

        int l_V = 0;
        while (l_V < l_Size)
        {
            int l_U = 0;
            while (l_U < l_Size)
            {
                const BlockId l_Current = l_Mask[static_cast<size_t>(l_V * l_Size + l_U)];
                if (l_Current == BlockId::Air)
                {
                    ++l_U;
                    continue;
                }

                int l_Width = 1;
                while (l_U + l_Width < l_Size && l_Mask[static_cast<size_t>(l_V * l_Size + l_U + l_Width)] == l_Current)
                {
                    ++l_Width;
                }

                int l_Height = 1;
                bool l_Done = false;
                while (l_V + l_Height < l_Size && !l_Done)
                {
                    for (int l_Test = 0; l_Test < l_Width; ++l_Test)
                    {
                        if (l_Mask[static_cast<size_t>((l_V + l_Height) * l_Size + l_U + l_Test)] != l_Current)
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
                        l_Mask[static_cast<size_t>((l_V + l_CoverV) * l_Size + l_U + l_CoverU)] = BlockId::Air;
                    }
                }

                glm::vec3 l_Origin{ 0.0f };
                glm::vec3 l_UDir{ 0.0f };
                glm::vec3 l_VDir{ 0.0f };
                glm::vec3 l_Normal{ 0.0f };

                if (face == BlockFace::East)
                {
                    l_Origin = glm::vec3{ static_cast<float>(l_W + 1), static_cast<float>(l_U), static_cast<float>(l_V) };
                    l_UDir = glm::vec3{ 0.0f, static_cast<float>(l_Width), 0.0f };
                    l_VDir = glm::vec3{ 0.0f, 0.0f, static_cast<float>(l_Height) };
                    l_Normal = glm::vec3{ 1.0f, 0.0f, 0.0f };
                }
                else if (face == BlockFace::West)
                {
                    l_Origin = glm::vec3{ static_cast<float>(l_Size - l_W - 1), static_cast<float>(l_U), static_cast<float>(l_V + l_Height) };
                    l_UDir = glm::vec3{ 0.0f, static_cast<float>(l_Width), 0.0f };
                    l_VDir = glm::vec3{ 0.0f, 0.0f, -static_cast<float>(l_Height) };
                    l_Normal = glm::vec3{ -1.0f, 0.0f, 0.0f };
                }
                else if (face == BlockFace::Top)
                {
                    l_Origin = glm::vec3{ static_cast<float>(l_U), static_cast<float>(l_Size - l_W), static_cast<float>(l_V) };
                    l_UDir = glm::vec3{ static_cast<float>(l_Width), 0.0f, 0.0f };
                    l_VDir = glm::vec3{ 0.0f, 0.0f, static_cast<float>(l_Height) };
                    l_Normal = glm::vec3{ 0.0f, 1.0f, 0.0f };
                }
                else if (face == BlockFace::Bottom)
                {
                    l_Origin = glm::vec3{ static_cast<float>(l_U), static_cast<float>(l_W), static_cast<float>(l_V + l_Height) };
                    l_UDir = glm::vec3{ static_cast<float>(l_Width), 0.0f, 0.0f };
                    l_VDir = glm::vec3{ 0.0f, 0.0f, -static_cast<float>(l_Height) };
                    l_Normal = glm::vec3{ 0.0f, -1.0f, 0.0f };
                }
                else if (face == BlockFace::North)
                {
                    l_Origin = glm::vec3{ static_cast<float>(l_U), static_cast<float>(l_V), static_cast<float>(l_W + 1) };
                    l_UDir = glm::vec3{ static_cast<float>(l_Width), 0.0f, 0.0f };
                    l_VDir = glm::vec3{ 0.0f, static_cast<float>(l_Height), 0.0f };
                    l_Normal = glm::vec3{ 0.0f, 0.0f, 1.0f };
                }
                else // South
                {
                    l_Origin = glm::vec3{ static_cast<float>(l_U), static_cast<float>(l_V + l_Height), static_cast<float>(l_Size - l_W - 1) };
                    l_UDir = glm::vec3{ static_cast<float>(l_Width), 0.0f, 0.0f };
                    l_VDir = glm::vec3{ 0.0f, -static_cast<float>(l_Height), 0.0f };
                    l_Normal = glm::vec3{ 0.0f, 0.0f, -1.0f };
                }

                const BlockFaceUV l_UVs = m_Atlas.GetFaceUVs(l_Current, face);
                EmitQuad(l_Origin, l_UDir, l_VDir, l_Normal, l_UVs, outMesh);

                l_U += l_Width;
            }
            ++l_V;
        }
    }
}

void ChunkMesher::EmitQuad(const glm::vec3& origin, const glm::vec3& uDir, const glm::vec3& vDir, const glm::vec3& normal, const BlockFaceUV& uvs, MeshedChunk& outMesh) const
{
    // Generate four vertices forming a quad using supplied orientation vectors.
    const glm::vec3 l_P0 = origin;
    const glm::vec3 l_P1 = origin + vDir;
    const glm::vec3 l_P2 = origin + uDir + vDir;
    const glm::vec3 l_P3 = origin + uDir;

    const std::array<Engine::Mesh::Vertex, 4> l_Vertices = {
        Engine::Mesh::Vertex{ l_P0, normal, uvs.m_UV00 },
        Engine::Mesh::Vertex{ l_P1, normal, uvs.m_UV01 },
        Engine::Mesh::Vertex{ l_P2, normal, uvs.m_UV11 },
        Engine::Mesh::Vertex{ l_P3, normal, uvs.m_UV10 },
    };

    const uint32_t l_StartIndex = static_cast<uint32_t>(outMesh.m_Vertices.size());
    outMesh.m_Vertices.insert(outMesh.m_Vertices.end(), l_Vertices.begin(), l_Vertices.end());

    const std::array<uint32_t, 6> l_QuadIndices = {
        l_StartIndex + 0, l_StartIndex + 1, l_StartIndex + 2,
        l_StartIndex + 2, l_StartIndex + 3, l_StartIndex + 0
    };

    outMesh.m_Indices.insert(outMesh.m_Indices.end(), l_QuadIndices.begin(), l_QuadIndices.end());
}