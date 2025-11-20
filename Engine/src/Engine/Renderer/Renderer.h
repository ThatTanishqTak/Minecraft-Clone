#pragma once

#include <glad/glad.h>

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

    private:
        static bool CompileShader(GLuint shaderId, const char* shaderSource);
        static bool LinkProgram(GLuint vertexShader, GLuint fragmentShader, GLuint programId);

    private:
        static GLuint s_VertexArrayObject;
        static GLuint s_VertexBufferObject;
        static GLuint s_ShaderProgram;
    };
}