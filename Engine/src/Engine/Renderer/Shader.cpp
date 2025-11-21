#include "Engine/Renderer/Shader.h"

#include "Engine/Core/Log.h"

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

            ENGINE_ERROR("Shader creation failed during compilation stage");

            return;
        }

        m_ProgramID = glCreateProgram();
        m_IsValid = LinkProgram(l_VertexShader, l_FragmentShader);

        glDeleteShader(l_VertexShader);
        glDeleteShader(l_FragmentShader);

        if (m_IsValid)
        {
            ENGINE_INFO("Shader program created successfully with ID {}", m_ProgramID);
        }
        else
        {
            ENGINE_ERROR("Shader program linking failed");
        }
    }

    Shader::~Shader()
    {
        // Free the shader program when no longer needed.
        if (m_ProgramID != 0)
        {
            glDeleteProgram(m_ProgramID);
            m_ProgramID = 0;
        }

        ENGINE_TRACE("Shader program destroyed");
    }

    void Shader::Bind() const
    {
        glUseProgram(m_ProgramID);
    }

    void Shader::Unbind() const
    {
        glUseProgram(0);
    }

    void Shader::SetMat4(const std::string& uniformName, const glm::mat4& matrix) const
    {
        const GLint l_Location = glGetUniformLocation(m_ProgramID, uniformName.c_str());
        if (l_Location == -1)
        {
            ENGINE_WARN("Uniform '{}' not found when setting mat4", uniformName);
         
            return;
        }
        glUniformMatrix4fv(l_Location, 1, GL_FALSE, &matrix[0][0]);
    }

    void Shader::SetInt(const std::string& uniformName, int value) const
    {
        const GLint l_Location = glGetUniformLocation(m_ProgramID, uniformName.c_str());
        if (l_Location == -1)
        {
            ENGINE_WARN("Uniform '{}' not found when setting int", uniformName);

            return;
        }
        glUniform1i(l_Location, value);
    }

    bool Shader::HasUniform(const std::string& uniformName) const
    {
        // Query without caching because the renderer uses this sparingly to branch its bindings.
        const GLint l_Location = glGetUniformLocation(m_ProgramID, uniformName.c_str());

        return l_Location != -1;
    }

    void Shader::BindUniformBlock(const std::string& blockName, GLuint bindingPoint) const
    {
        const GLuint l_BlockIndex = glGetUniformBlockIndex(m_ProgramID, blockName.c_str());
        if (l_BlockIndex != GL_INVALID_INDEX)
        {
            glUniformBlockBinding(m_ProgramID, l_BlockIndex, bindingPoint);
            ENGINE_TRACE("Uniform block '{}' bound to point {}", blockName, bindingPoint);
        }
        else
        {
            ENGINE_WARN("Uniform block '{}' not found", blockName);
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
            ENGINE_ERROR("Shader compile error: {}", l_Message);

            return false;
        }

        ENGINE_TRACE("Shader stage {} compiled successfully", shaderID);

        return true;
    }

    bool Shader::LinkProgram(GLuint vertexShader, GLuint fragmentShader)
    {
        glAttachShader(m_ProgramID, vertexShader);
        glAttachShader(m_ProgramID, fragmentShader);
        glLinkProgram(m_ProgramID);

        GLint l_IsLinked = 0;
        glGetProgramiv(m_ProgramID, GL_LINK_STATUS, &l_IsLinked);
        if (l_IsLinked == GL_FALSE)
        {
            GLint l_LogLength = 0;
            glGetProgramiv(m_ProgramID, GL_INFO_LOG_LENGTH, &l_LogLength);

            std::string l_Message(static_cast<size_t>(l_LogLength), '\0');
            glGetProgramInfoLog(m_ProgramID, l_LogLength, nullptr, l_Message.data());
            ENGINE_ERROR("Program link error: {}", l_Message);

            return false;
        }

        ENGINE_TRACE("Shader program {} linked successfully", m_ProgramID);

        return true;
    }
}