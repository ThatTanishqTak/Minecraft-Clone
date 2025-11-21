#pragma once

#include <string>
#include <glm/glm.hpp>
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

        // Convenience helpers for binding uniforms.
        void SetMat4(const std::string& uniformName, const glm::mat4& matrix) const;
        void SetInt(const std::string& uniformName, int value) const;
        void BindUniformBlock(const std::string& blockName, GLuint bindingPoint) const;

        GLuint GetProgramId() const { return m_ProgramID; }

    private:
        bool CompileShader(GLuint shaderID, const char* shaderSource);
        bool LinkProgram(GLuint vertexShader, GLuint fragmentShader);

    private:
        GLuint m_ProgramID = 0;
        bool m_IsValid = false;
    };
}