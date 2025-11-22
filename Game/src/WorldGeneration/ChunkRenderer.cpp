#include "ChunkRenderer.h"

#include "Engine/Core/Log.h"

void ChunkRenderer::UpdateMesh(const std::shared_ptr<Engine::Mesh>& meshBuffer)
{
    // Cache the shared mesh buffer for this chunk so it can be reused by the pool.
    m_Mesh = meshBuffer;

    GAME_TRACE("ChunkRenderer mesh updated with shared buffer ({} indices)", m_Mesh != nullptr ? m_Mesh->GetIndexCount() : 0);
}

void ChunkRenderer::Render(const glm::mat4& modelMatrix) const
{
    if (m_Mesh == nullptr)
    {
        GAME_WARN("ChunkRenderer::Render called with no mesh available");
        return;
    }

    // Submit the mesh with the active atlas texture so the shader can sample block surfaces.
    Engine::Renderer::SubmitMesh(*m_Mesh, modelMatrix, m_Texture);

    //GAME_TRACE("ChunkRenderer submitted mesh for rendering");
}