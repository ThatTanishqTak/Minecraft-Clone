#pragma once

#include <memory>

#include <glad/glad.h>

#include "Engine/Renderer/Buffers.h"
#include "Engine/Renderer/RenderQueue.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/World/ChunkMesher.h"

namespace Engine
{
    // Keep GPU state for a chunk in this component.
    class ChunkRenderComponent
    {
    public:
        ChunkRenderComponent();
        ~ChunkRenderComponent();

        void SetShader(const std::shared_ptr<Shader>& shaderProgram);
        void SetTexture(GLuint textureId);
        void UpdateMesh(const ChunkMesh& meshData);
        void Enqueue(RenderQueue& renderQueue) const;

    private:
        void DestroyBuffers();

    private:
        GLuint m_VertexArrayObject = 0;
        std::shared_ptr<VertexBuffer> m_VertexBuffer;
        std::shared_ptr<IndexBuffer> m_IndexBuffer;
        std::shared_ptr<Shader> m_ShaderProgram;
        GLuint m_TextureId = 0;
        GLsizei m_ElementCount = 0;
    };
}