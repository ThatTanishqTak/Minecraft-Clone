#include "Engine/Renderer/Buffers.h"

#include "Engine/Core/Log.h"

namespace Engine
{
    VertexBuffer::VertexBuffer(const void* data, size_t sizeInBytes, GLenum usage)
    {
        // Generate and upload vertex data to the GPU.
        glGenBuffers(1, &m_BufferId);
        Bind();
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(sizeInBytes), data, usage);

        Unbind();

        ENGINE_TRACE("VertexBuffer created with {} bytes", sizeInBytes);
    }

    VertexBuffer::~VertexBuffer()
    {
        // Release GPU memory when the buffer is destroyed.
        if (m_BufferId != 0)
        {
            glDeleteBuffers(1, &m_BufferId);
            m_BufferId = 0;
        }

        ENGINE_TRACE("VertexBuffer destroyed");
    }

    void VertexBuffer::Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_BufferId);
    }

    void VertexBuffer::Unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    IndexBuffer::IndexBuffer(const void* data, size_t count, GLenum type, GLenum usage) : m_Count(count), m_IndexType(type)
    {
        // Generate and upload index data to the GPU.
        glGenBuffers(1, &m_BufferId);
        Bind();
        size_t l_BytesPerIndex = sizeof(unsigned int);
        if (type == GL_UNSIGNED_SHORT)
        {
            l_BytesPerIndex = sizeof(unsigned short);
        }
        else if (type == GL_UNSIGNED_BYTE)
        {
            l_BytesPerIndex = sizeof(unsigned char);
        }

        const GLsizeiptr l_SizeInBytes = static_cast<GLsizeiptr>(count * l_BytesPerIndex);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, l_SizeInBytes, data, usage);
        Unbind();

        ENGINE_TRACE("IndexBuffer created with {} indices", count);
    }

    IndexBuffer::~IndexBuffer()
    {
        // Release GPU memory when the buffer is destroyed.
        if (m_BufferId != 0)
        {
            glDeleteBuffers(1, &m_BufferId);
            m_BufferId = 0;
        }

        ENGINE_TRACE("IndexBuffer destroyed");
    }

    void IndexBuffer::Bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
    }

    void IndexBuffer::Unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}