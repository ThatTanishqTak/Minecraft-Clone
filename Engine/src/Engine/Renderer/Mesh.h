#pragma once

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Engine/Renderer/Buffers.h"

namespace Engine
{
    // Represents a drawable mesh with vertex/index buffers and a vertex array object.
    class Mesh
    {
    public:
        struct Vertex
        {
            glm::vec3 m_Position;
            glm::vec3 m_Normal;
            glm::vec3 m_Color;
        };

        Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
        ~Mesh();

        void Bind() const;
        void Unbind() const;

        GLsizei GetIndexCount() const { return m_IndexCount; }
        GLuint GetVertexArray() const { return m_VertexArrayObject; }

    private:
        void InitializeBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    private:
        GLuint m_VertexArrayObject = 0;
        std::shared_ptr<VertexBuffer> m_VertexBuffer;
        std::shared_ptr<IndexBuffer> m_IndexBuffer;
        GLsizei m_IndexCount = 0;
    };
}