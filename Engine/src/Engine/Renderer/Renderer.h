#pragma once

#include <memory>
#include <glad/glad.h>

#include "Engine/Renderer/Buffers.h"
#include "Engine/Renderer/RenderQueue.h"
#include "Engine/Renderer/RendererCommands.h"
#include "Engine/Renderer/Shader.h"

namespace Engine
{
    // Utility class encapsulating minimal rendering setup used by the game layer.
    class Renderer
    {
    public:
        static bool Initialize();
        static void Shutdown();

        static void BeginFrame();
        static void EndFrame();
        static void DrawPlaceholderGeometry();
        
        static RenderQueue* GetRenderQueue() { return s_RenderQueue.get(); }

    private:
        static GLuint s_VertexArrayObject;
        static std::shared_ptr<VertexBuffer> s_PlaceholderVertexBuffer;
        static std::shared_ptr<IndexBuffer> s_PlaceholderIndexBuffer;
        static std::shared_ptr<Shader> s_PlaceholderShader;
        static std::unique_ptr<RenderQueue> s_RenderQueue;
    };
}