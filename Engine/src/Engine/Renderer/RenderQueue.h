#pragma once

#include <memory>
#include <vector>

#include <glad/glad.h>

namespace Engine
{
    class Shader;
    class IndexBuffer;

    // Description of a single draw call.
    struct RenderCommand
    {
        GLuint VertexArrayObject = 0;
        std::shared_ptr<Shader> ShaderProgram;
        GLenum PrimitiveType = GL_TRIANGLES;
        GLsizei ElementCount = 0;
        bool UseIndices = false;
        std::shared_ptr<IndexBuffer> IndexBufferObject;
    };

    // Queues draw commands for batched submission.
    class RenderQueue
    {
    public:
        void Submit(const RenderCommand& command);
        void Flush();

    private:
        std::vector<RenderCommand> m_CommandBuffer;
    };
}