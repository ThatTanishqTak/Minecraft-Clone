#include "Engine/Renderer/Mesh.h"

#include <cstddef>

namespace Engine
{
    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        // Build the GPU resources immediately so the mesh is ready for rendering.
        InitializeBuffers(vertices, indices);
    }

    Mesh::~Mesh()
    {
        // Release OpenGL resources when the mesh lifetime ends.
        if (m_VertexArrayObject != 0)
        {
            glDeleteVertexArrays(1, &m_VertexArrayObject);
            m_VertexArrayObject = 0;
        }
    }

    void Mesh::Bind() const
    {
        glBindVertexArray(m_VertexArrayObject);
    }

    void Mesh::Unbind() const
    {
        glBindVertexArray(0);
    }

    void Mesh::InitializeBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        m_IndexCount = static_cast<GLsizei>(indices.size());

        glGenVertexArrays(1, &m_VertexArrayObject);
        glBindVertexArray(m_VertexArrayObject);

        m_VertexBuffer = std::make_shared<VertexBuffer>(vertices.data(), sizeof(Vertex) * vertices.size(), GL_STATIC_DRAW);
        m_IndexBuffer = std::make_shared<IndexBuffer>(indices.data(), indices.size(), GL_UNSIGNED_INT, GL_STATIC_DRAW);

        m_VertexBuffer->Bind();

        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, m_Position)));

        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, m_Normal)));

        // UV
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, m_UV)));

        m_VertexBuffer->Unbind();
        glBindVertexArray(0);
    }
}