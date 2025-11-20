#pragma once

#include <memory>
#include <glad/glad.h>

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Buffers.h"
#include "Engine/Renderer/RenderQueue.h"
#include "Engine/Renderer/RendererCommands.h"
#include "Engine/Renderer/Shader.h"

namespace Engine
{
    // Utility class encapsulating minimal rendering setup used by the game layer.
    class ENGINE_API Renderer
    {
    public:
        static bool Initialize();
        static void Shutdown();

        static void BeginFrame();
        static void EndFrame();
        static void DrawPlaceholderGeometry();

        // Accessor is declared here and defined in the implementation file to
        // ensure the exported RenderQueue symbol is referenced from a single
        // translation unit when building or consuming the Engine DLL.
        static RenderQueue* GetRenderQueue();

    private:
        static GLuint s_VertexArrayObject;
        static std::shared_ptr<VertexBuffer> s_PlaceholderVertexBuffer;
        static std::shared_ptr<IndexBuffer> s_PlaceholderIndexBuffer;
        static std::shared_ptr<Shader> s_PlaceholderShader;
        static std::unique_ptr<RenderQueue> s_RenderQueue;
    };
}