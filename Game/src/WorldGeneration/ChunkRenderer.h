#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Texture2D.h"
#include "ChunkMesher.h"

// Owns GPU buffers for a chunk mesh and submits them to the renderer each frame.
class ChunkRenderer
{
public:
    ChunkRenderer() = default;

    void UpdateMesh(const std::shared_ptr<Engine::Mesh>& meshBuffer);
    void SetTexture(const Engine::Texture2D* texture) { m_Texture = texture; }
    // Allow callers to check whether a GPU mesh is ready before issuing draw calls.
    bool HasMesh() const { return m_Mesh != nullptr; }
    void Render(const glm::mat4& modelMatrix) const;

private:
    std::shared_ptr<Engine::Mesh> m_Mesh;
    const Engine::Texture2D* m_Texture = nullptr;
};