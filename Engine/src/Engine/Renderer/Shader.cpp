#include "Engine/Renderer/Shader.h"

#include <iostream>

namespace Engine
{
    Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource)
    {
        // Compile and link shader stages into a program.
        GLuint l_VertexShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint l_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        m_IsValid = CompileShader(l_VertexShader, vertexSource.c_str()) && CompileShader(l_FragmentShader, fragmentSource.c_str());

        if (!m_IsValid)
        {
            // Cleanup happens after compilation failure to avoid leaks.
            glDeleteShader(l_VertexShader);
            glDeleteShader(l_FragmentShader);

            return;
        }

        m_ProgramId = glCreateProgram();
        m_IsValid = LinkProgram(l_VertexShader, l_FragmentShader);

        glDeleteShader(l_VertexShader);
        glDeleteShader(l_FragmentShader);
    }

    Shader::~Shader()
    {
        // Free the shader program when no longer needed.
        if (m_ProgramId != 0)
        {
            glDeleteProgram(m_ProgramId);
            m_ProgramId = 0;
        }
    }

    void Shader::Bind() const
    {
        glUseProgram(m_ProgramId);
    }

    void Shader::Unbind() const
    {
        glUseProgram(0);
    }

    void Shader::SetMat4(const std::string& uniformName, const glm::mat4& matrix) const
    {
        const GLint l_Location = glGetUniformLocation(m_ProgramId, uniformName.c_str());
        glUniformMatrix4fv(l_Location, 1, GL_FALSE, &matrix[0][0]);
    }

    void Shader::BindUniformBlock(const std::string& blockName, GLuint bindingPoint) const
    {
        const GLuint l_BlockIndex = glGetUniformBlockIndex(m_ProgramId, blockName.c_str());
        if (l_BlockIndex != GL_INVALID_INDEX)
        {
            glUniformBlockBinding(m_ProgramId, l_BlockIndex, bindingPoint);
        }
    }

    bool Shader::CompileShader(GLuint shaderID, const char* shaderSource)
    {
        const char* l_Source = shaderSource;
        glShaderSource(shaderID, 1, &l_Source, nullptr);
        glCompileShader(shaderID);

        GLint l_IsCompiled = 0;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &l_IsCompiled);
        if (l_IsCompiled == GL_FALSE)
        {
            GLint l_LogLength = 0;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &l_LogLength);

            std::string l_Message(static_cast<size_t>(l_LogLength), '\0');
            glGetShaderInfoLog(shaderID, l_LogLength, nullptr, l_Message.data());
            std::cout << "Shader compile error: " << l_Message << std::endl;

            return false;
        }

        return true;
    }

    bool Shader::LinkProgram(GLuint vertexShader, GLuint fragmentShader)
    {
        glAttachShader(m_ProgramId, vertexShader);
        glAttachShader(m_ProgramId, fragmentShader);
        glLinkProgram(m_ProgramId);

        GLint l_IsLinked = 0;
        glGetProgramiv(m_ProgramId, GL_LINK_STATUS, &l_IsLinked);
        if (l_IsLinked == GL_FALSE)
        {
            GLint l_LogLength = 0;
            glGetProgramiv(m_ProgramId, GL_INFO_LOG_LENGTH, &l_LogLength);

            std::string l_Message(static_cast<size_t>(l_LogLength), '\0');
            glGetProgramInfoLog(m_ProgramId, l_LogLength, nullptr, l_Message.data());
            std::cout << "Program link error: " << l_Message << std::endl;

            return false;
        }

        return true;
    }
}