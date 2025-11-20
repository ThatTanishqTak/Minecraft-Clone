#include "Engine/Renderer/ChunkRenderComponent.h"

namespace Engine
{
    ChunkRenderComponent::ChunkRenderComponent()
    {
        // Allocate a vertex array so it can bind buffers cleanly.
        glGenVertexArrays(1, &m_VertexArrayObject);
    }

    ChunkRenderComponent::~ChunkRenderComponent()
    {
        DestroyBuffers();
    }

    void ChunkRenderComponent::SetShader(const std::shared_ptr<Shader>& shaderProgram)
    {
        m_ShaderProgram = shaderProgram;
    }

    void ChunkRenderComponent::UpdateMesh(const ChunkMesh& meshData)
    {
        if (m_VertexArrayObject == 0)
        {
            glGenVertexArrays(1, &m_VertexArrayObject);
        }

        glBindVertexArray(m_VertexArrayObject);

        // Refresh the vertex buffer with the new mesh data.
        m_VertexBuffer = std::make_shared<VertexBuffer>(meshData.Vertices.data(), meshData.Vertices.size() * sizeof(ChunkVertex), GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), reinterpret_cast<void*>(sizeof(float) * 3));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), reinterpret_cast<void*>(sizeof(float) * 6));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), reinterpret_cast<void*>(sizeof(float) * 9));
        glEnableVertexAttribArray(3);

        // Refresh the index buffer so draw calls use the right indices.
        m_IndexBuffer = std::make_shared<IndexBuffer>(meshData.Indices.data(), meshData.Indices.size(), GL_UNSIGNED_INT, GL_DYNAMIC_DRAW);
        m_ElementCount = static_cast<GLsizei>(meshData.Indices.size());

        glBindVertexArray(0);
    }

    void ChunkRenderComponent::Enqueue(RenderQueue& renderQueue) const
    {
        if (m_ShaderProgram == nullptr || m_VertexArrayObject == 0 || m_ElementCount == 0)
        {
            return;
        }

        RenderCommand l_Command = {};
        l_Command.VertexArrayObject = m_VertexArrayObject;
        l_Command.ShaderProgram = m_ShaderProgram;
        l_Command.PrimitiveType = GL_TRIANGLES;
        l_Command.ElementCount = m_ElementCount;
        l_Command.UseIndices = true;
        l_Command.IndexBufferObject = m_IndexBuffer;
        l_Command.UniformCallback = nullptr;

        renderQueue.Submit(l_Command);
    }

    void ChunkRenderComponent::DestroyBuffers()
    {
        // Release GPU objects before destruction to avoid leaks.
        m_VertexBuffer.reset();
        m_IndexBuffer.reset();
        m_ShaderProgram.reset();

        if (m_VertexArrayObject != 0)
        {
            glDeleteVertexArrays(1, &m_VertexArrayObject);
            m_VertexArrayObject = 0;
        }

        m_ElementCount = 0;
    }
}