#include "Engine/Renderer/Renderer.h"

#include <array>
#include <iostream>
#include <string>

namespace Engine
{
    GLuint Renderer::s_VertexArrayObject = 0;
    std::shared_ptr<VertexBuffer> Renderer::s_PlaceholderVertexBuffer = nullptr;
    std::shared_ptr<IndexBuffer> Renderer::s_PlaceholderIndexBuffer = nullptr;
    std::shared_ptr<Shader> Renderer::s_PlaceholderShader = nullptr;

    bool Renderer::Initialize()
    {
        // Minimal shader sources for placeholder geometry rendering.
        const char* l_VertexSource = R"(
            #version 430 core
            layout (location = 0) in vec3 aPos;

            void main()
            {
                gl_Position = vec4(aPos, 1.0);
            }
        )";

        const char* l_FragmentSource = R"(
            #version 430 core
            out vec4 FragColor;

            void main()
            {
                FragColor = vec4(0.5, 0.5, 0.5, 1.0);
            }
        )";

        s_PlaceholderShader = std::make_shared<Shader>(l_VertexSource, l_FragmentSource);
        if (s_PlaceholderShader == nullptr || !s_PlaceholderShader->IsValid())
        {
            std::cout << "Failed to create placeholder shader" << std::endl;

            return false;
        }

        // Two triangles forming a simple quad footprint in normalized device coordinates.
        std::array<float, 18> l_Vertices =
        {
            // First triangle
            -0.5f, -0.5f, 0.0f,  // bottom-left
             0.5f, -0.5f, 0.0f,  // bottom-right
             0.5f,  0.5f, 0.0f,  // top-right

             // Second triangle
              0.5f,  0.5f, 0.0f,  // top-right
             -0.5f,  0.5f, 0.0f,  // top-left
             -0.5f, -0.5f, 0.0f   // bottom-left
        };

        std::array<unsigned int, 6> l_Indices =
        {
            0, 1, 2,
            3, 4, 5
        };

        s_PlaceholderVertexBuffer = std::make_shared<VertexBuffer>(l_Vertices.data(), sizeof(float) * l_Vertices.size(), GL_STATIC_DRAW);
        s_PlaceholderIndexBuffer = std::make_shared<IndexBuffer>(l_Indices.data(), l_Indices.size(), GL_UNSIGNED_INT, GL_STATIC_DRAW);

        glGenVertexArrays(1, &s_VertexArrayObject);
        glBindVertexArray(s_VertexArrayObject);

        s_PlaceholderVertexBuffer->Bind();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);
        s_PlaceholderVertexBuffer->Unbind();

        glBindVertexArray(0);

        return true;
    }

    void Renderer::Shutdown()
    {
        // Release GPU resources before tearing down OpenGL state.
        s_PlaceholderShader.reset();
        s_PlaceholderVertexBuffer.reset();
        s_PlaceholderIndexBuffer.reset();

        if (s_VertexArrayObject != 0)
        {
            glDeleteVertexArrays(1, &s_VertexArrayObject);
            s_VertexArrayObject = 0;
        }
    }

    void Renderer::BeginFrame()
    {
        // Ensure the viewport matches the framebuffer each frame.
        GLint l_Viewport[4] = { 0, 0, 0, 0 };
        glGetIntegerv(GL_VIEWPORT, l_Viewport);
        RendererCommands::SetViewport(l_Viewport[0], l_Viewport[1], l_Viewport[2], l_Viewport[3]);

        // Establish clear color and depth state to keep frames deterministic.
        RendererCommands::EnableDepthTest();
        RendererCommands::SetClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        RendererCommands::Clear();
    }

    void Renderer::EndFrame()
    {

    }

    void Renderer::DrawPlaceholderGeometry()
    {
        if (s_PlaceholderShader == nullptr || s_PlaceholderIndexBuffer == nullptr)
        {
            return;
        }
    }
}