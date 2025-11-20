#pragma once

#include <cstddef>
#include <glad/glad.h>

namespace Engine
{
    // Abstraction for an OpenGL vertex buffer.
    class VertexBuffer
    {
    public:
        VertexBuffer(const void* data, size_t sizeInBytes, GLenum usage);
        ~VertexBuffer();

        void Bind() const;
        void Unbind() const;

        GLuint GetId() const { return m_BufferId; }

    private:
        GLuint m_BufferId = 0;
    };

    // Abstraction for an OpenGL index buffer.
    class IndexBuffer
    {
    public:
        IndexBuffer(const void* data, size_t count, GLenum type, GLenum usage);
        ~IndexBuffer();

        void Bind() const;
        void Unbind() const;

        GLuint GetId() const { return m_BufferId; }
        GLsizei GetCount() const { return static_cast<GLsizei>(m_Count); }
        GLenum GetIndexType() const { return m_IndexType; }

    private:
        GLuint m_BufferId = 0;
        size_t m_Count = 0;
        GLenum m_IndexType = 0;
    };
}