#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Engine/Renderer/Mesh.h"
#include "Block.h"
#include "Chunk.h"
#include "TextureAtlas.h"

// Stores vertex/index buffers for a meshed chunk ready to upload to the renderer.
struct MeshedChunk
{
    std::vector<Engine::Mesh::Vertex> m_Vertices;
    std::vector<uint32_t> m_Indices;
};

// Implements greedy meshing to reduce the number of faces emitted for a chunk.
class ChunkMesher
{
public:
    explicit ChunkMesher(const TextureAtlas& atlas);

    MeshedChunk Mesh(const Chunk& chunk) const;

private:
    void BuildFaceQuads(const Chunk& chunk, BlockFace face, MeshedChunk& outMesh) const;
    void EmitQuad(const glm::vec3& origin, const glm::vec3& uDirection, const glm::vec3& vDirection, const glm::vec3& normal, const BlockFaceUV& UVs, MeshedChunk& outMesh) const;

private:
    const TextureAtlas& m_Atlas;
};