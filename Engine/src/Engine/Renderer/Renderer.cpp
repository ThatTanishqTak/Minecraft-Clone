#include "Engine/Renderer/Renderer.h"

#include <array>
#include <iostream>
#include <string>

namespace Engine
{
    GLuint Renderer::s_VertexArrayObject = 0;
    GLuint Renderer::s_VertexBufferObject = 0;
    GLuint Renderer::s_ShaderProgram = 0;

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

        GLuint l_VertexShader = glCreateShader(GL_VERTEX_SHADER);
        if (!CompileShader(l_VertexShader, l_VertexSource))
        {
            std::cout << "Failed to compile vertex shader" << std::endl;

            return false;
        }

        GLuint l_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        if (!CompileShader(l_FragmentShader, l_FragmentSource))
        {
            std::cout << "Failed to compile fragment shader" << std::endl;
            glDeleteShader(l_VertexShader);

            return false;
        }

        s_ShaderProgram = glCreateProgram();
        if (!LinkProgram(l_VertexShader, l_FragmentShader, s_ShaderProgram))
        {
            std::cout << "Failed to link shader program" << std::endl;
            glDeleteShader(l_VertexShader);
            glDeleteShader(l_FragmentShader);

            return false;
        }

        glDeleteShader(l_VertexShader);
        glDeleteShader(l_FragmentShader);

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

        glGenVertexArrays(1, &s_VertexArrayObject);
        glGenBuffers(1, &s_VertexBufferObject);

        glBindVertexArray(s_VertexArrayObject);

        glBindBuffer(GL_ARRAY_BUFFER, s_VertexBufferObject);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * l_Vertices.size(), l_Vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        return true;
    }

    void Renderer::Shutdown()
    {
        if (s_VertexBufferObject != 0)
        {
            glDeleteBuffers(1, &s_VertexBufferObject);
            s_VertexBufferObject = 0;
        }

        if (s_VertexArrayObject != 0)
        {
            glDeleteVertexArrays(1, &s_VertexArrayObject);
            s_VertexArrayObject = 0;
        }

        if (s_ShaderProgram != 0)
        {
            glDeleteProgram(s_ShaderProgram);
            s_ShaderProgram = 0;
        }
    }

    void Renderer::BeginFrame()
    {
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::EndFrame()
    {
        // Future scope: add post-processing or debug overlays here.
    }

    void Renderer::DrawPlaceholderGeometry()
    {
        glUseProgram(s_ShaderProgram);
        glBindVertexArray(s_VertexArrayObject);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }

    bool Renderer::CompileShader(GLuint shaderId, const char* shaderSource)
    {
        const char* l_Source = shaderSource;
        glShaderSource(shaderId, 1, &l_Source, nullptr);
        glCompileShader(shaderId);

        GLint l_IsCompiled = 0;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &l_IsCompiled);
        if (l_IsCompiled == GL_FALSE)
        {
            GLint l_LogLength = 0;
            glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &l_LogLength);

            std::string l_Message(static_cast<size_t>(l_LogLength), '\0');
            glGetShaderInfoLog(shaderId, l_LogLength, nullptr, l_Message.data());
            std::cout << "Shader compile error: " << l_Message << std::endl;

            return false;
        }

        return true;
    }

    bool Renderer::LinkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint programId)
    {
        glAttachShader(programId, vertexShader);
        glAttachShader(programId, fragmentShader);
        glLinkProgram(programId);

        GLint l_IsLinked = 0;
        glGetProgramiv(programId, GL_LINK_STATUS, &l_IsLinked);
        if (l_IsLinked == GL_FALSE)
        {
            GLint l_LogLength = 0;
            glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &l_LogLength);

            std::string l_Message(static_cast<size_t>(l_LogLength), '\0');
            glGetProgramInfoLog(programId, l_LogLength, nullptr, l_Message.data());
            std::cout << "Program link error: " << l_Message << std::endl;

            return false;
        }

        return true;
    }
}