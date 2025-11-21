#include "Engine/Renderer/Texture2D.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <array>
#include <iostream>

namespace Engine
{
    Texture2D::Texture2D(const std::string& filePath)
    {
        int l_Width = 0;
        int l_Height = 0;
        int l_Channels = 0;

        stbi_uc* l_Data = stbi_load(filePath.c_str(), &l_Width, &l_Height, &l_Channels, STBI_rgb_alpha);
        if (l_Data == nullptr)
        {
            std::cout << "Failed to load texture: " << filePath << std::endl;
            m_IsValid = false;

            return;
        }

        l_Channels = 4; // Force RGBA from stb_image.
        InitializeTexture(l_Width, l_Height, l_Channels, l_Data);
        stbi_image_free(l_Data);
    }

    Texture2D::Texture2D(int width, int height, int channels, const unsigned char* data)
    {
        InitializeTexture(width, height, channels, data);
    }

    Texture2D::~Texture2D()
    {
        if (m_TextureId != 0)
        {
            glDeleteTextures(1, &m_TextureId);
            m_TextureId = 0;
        }
    }

    void Texture2D::Bind(unsigned int slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_2D, m_TextureId);
    }

    void Texture2D::InitializeTexture(int width, int height, int channels, const unsigned char* data)
    {
        m_Width = width;
        m_Height = height;
        m_Channels = channels;

        glGenTextures(1, &m_TextureId);
        glBindTexture(GL_TEXTURE_2D, m_TextureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        GLint l_Format = channels == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, l_Format, m_Width, m_Height, 0, l_Format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        m_IsValid = m_TextureId != 0;
    }
}