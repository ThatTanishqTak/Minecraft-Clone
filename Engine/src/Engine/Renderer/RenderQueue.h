#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <glad/glad.h>

#include "Engine/Core/Core.h"

namespace Engine
{
    class Shader;
    class IndexBuffer;

    // Description of a single draw call.
    struct ENGINE_API RenderCommand
    {
        GLuint VertexArrayObject = 0;
        std::shared_ptr<Shader> ShaderProgram;
        GLenum PrimitiveType = GL_TRIANGLES;
        GLsizei ElementCount = 0;
        bool UseIndices = false;
        std::shared_ptr<IndexBuffer> IndexBufferObject;
        GLuint TextureId = 0;
        std::function<void(const Shader&)> UniformCallback;
    };

    // Queues draw commands for batched submission.
    class ENGINE_API RenderQueue
    {
    public:
        void Submit(const RenderCommand& command);
        void Flush();

    private:
        std::vector<RenderCommand> m_CommandBuffer;
    };
}