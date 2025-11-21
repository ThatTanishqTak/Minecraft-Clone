#include "ChunkRenderer.h"

#include "Engine/Core/Log.h"

void ChunkRenderer::UpdateMesh(const MeshedChunk& meshedChunk)
{
    // Replace the existing mesh with freshly generated geometry from the mesher.
    m_Mesh = std::make_unique<Engine::Mesh>(meshedChunk.m_Vertices, meshedChunk.m_Indices);

    GAME_TRACE("ChunkRenderer mesh updated with {} vertices and {} indices", meshedChunk.m_Vertices.size(), meshedChunk.m_Indices.size());
}

void ChunkRenderer::Render(const glm::mat4& modelMatrix, const Engine::Texture2D* texture) const
{
    if (m_Mesh == nullptr)
    {
        GAME_WARN("ChunkRenderer::Render called with no mesh available");
        return;
    }

    Engine::Renderer::SubmitMesh(*m_Mesh, modelMatrix, texture);

    GAME_TRACE("ChunkRenderer submitted mesh for rendering");
}