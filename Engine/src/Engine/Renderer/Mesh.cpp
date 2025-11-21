#include "Engine/Renderer/Mesh.h"

#include "Engine/Core/Log.h"

#include <cstddef>

namespace Engine
{
    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        // Build the GPU resources immediately so the mesh is ready for rendering.
        InitializeBuffers(vertices, indices);

        ENGINE_TRACE("Mesh created with {} vertices and {} indices", vertices.size(), indices.size());
    }

    Mesh::~Mesh()
    {
        // Release OpenGL resources when the mesh lifetime ends.
        if (m_VertexArrayObject != 0)
        {
            glDeleteVertexArrays(1, &m_VertexArrayObject);
            m_VertexArrayObject = 0;
        }

        ENGINE_TRACE("Mesh resources released");
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

        // Position attribute (location = 0)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, m_Position)));

        // Normal attribute (location = 1)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, m_Normal)));

        // Color attribute (location = 2) ensures vertex colors arrive at the shader.
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, m_Color)));

        m_VertexBuffer->Unbind();

        // Rebind the index buffer while the VAO is active so the draw call uses the correct element array.
        m_IndexBuffer->Bind();
        glBindVertexArray(0);

        ENGINE_TRACE("Mesh buffers initialized and vertex array configured");
    }
}