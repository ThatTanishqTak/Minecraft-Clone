#include "Engine/Renderer/RenderQueue.h"

#include "Engine/Renderer/Buffers.h"
#include "Engine/Renderer/Shader.h"

namespace Engine
{
    void RenderQueue::Submit(const RenderCommand& command)
    {
        // Store the command so it can be executed during the flush step.
        m_CommandBuffer.push_back(command);
    }

    void RenderQueue::Flush()
    {
        // Execute every buffered draw call in order.
        for (const RenderCommand& it_Command : m_CommandBuffer)
        {
            if (it_Command.ShaderProgram == nullptr || it_Command.VertexArrayObject == 0)
            {
                continue;
            }

            it_Command.ShaderProgram->Bind();
            glBindVertexArray(it_Command.VertexArrayObject);

            // Bind an optional texture if the command requests one.
            if (it_Command.TextureId != 0)
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, it_Command.TextureId);
            }

            // Allow callers to push uniform values such as sampler bindings.
            if (it_Command.UniformCallback)
            {
                it_Command.UniformCallback(*it_Command.ShaderProgram);
            }

            if (it_Command.UseIndices && it_Command.IndexBufferObject != nullptr)
            {
                it_Command.IndexBufferObject->Bind();
                glDrawElements(it_Command.PrimitiveType, it_Command.ElementCount, it_Command.IndexBufferObject->GetIndexType(), nullptr);
            }
            else
            {
                glDrawArrays(it_Command.PrimitiveType, 0, it_Command.ElementCount);
            }

            glBindVertexArray(0);
        }

        // Clear the buffer so the next frame starts empty.
        m_CommandBuffer.clear();
    }
}