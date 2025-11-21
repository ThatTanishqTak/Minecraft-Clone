#pragma once

#include <string>
#include <glad/glad.h>

namespace Engine
{
    // Lightweight 2D texture wrapper for sampling atlas data in shaders.
    class Texture2D
    {
    public:
        Texture2D(const std::string& filePath);
        Texture2D(int width, int height, int channels, const unsigned char* data);
        ~Texture2D();

        bool IsValid() const { return m_IsValid; }

        void Bind(unsigned int slot = 0) const;

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        int GetChannels() const { return m_Channels; }

    private:
        void InitializeTexture(int width, int height, int channels, const unsigned char* data);

    private:
        GLuint m_TextureId = 0;
        int m_Width = 0;
        int m_Height = 0;
        int m_Channels = 0;
        bool m_IsValid = false;
    };
}