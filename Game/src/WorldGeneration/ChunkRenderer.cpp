#include "ChunkRenderer.h"

void ChunkRenderer::UpdateMesh(const MeshedChunk& meshedChunk)
{
    // Replace the existing mesh with freshly generated geometry from the mesher.
    m_Mesh = std::make_unique<Engine::Mesh>(meshedChunk.m_Vertices, meshedChunk.m_Indices);
}

void ChunkRenderer::Render(const glm::mat4& modelMatrix, const Engine::Texture2D* texture) const
{
    if (m_Mesh == nullptr)
    {
        return;
    }

    Engine::Renderer::SubmitMesh(*m_Mesh, modelMatrix, texture);
}