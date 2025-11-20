#pragma once

#include <string>
#include <glad/glad.h>

namespace Engine
{
    // Simple shader wrapper handling compilation and binding.
    class Shader
    {
    public:
        Shader(const std::string& vertexSource, const std::string& fragmentSource);
        ~Shader();

        bool IsValid() const { return m_IsValid; }

        void Bind() const;
        void Unbind() const;

        GLuint GetProgramId() const { return m_ProgramId; }

    private:
        bool CompileShader(GLuint shaderId, const char* shaderSource);
        bool LinkProgram(GLuint vertexShader, GLuint fragmentShader);

    private:
        GLuint m_ProgramId = 0;
        bool m_IsValid = false;
    };
}