#pragma once

#include <memory>
#include <glm/glm.hpp>

#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/Renderer.h"
#include "ChunkMesher.h"
#include "TextureAtlas.h"

// Owns GPU buffers for a chunk mesh and submits them to the renderer each frame.
class ChunkRenderer
{
public:
    ChunkRenderer() = default;

    void UpdateMesh(const MeshedChunk& meshedChunk);
    void Render(const glm::mat4& modelMatrix, const Engine::Texture2D* texture) const;

private:
    std::unique_ptr<Engine::Mesh> m_Mesh;
};