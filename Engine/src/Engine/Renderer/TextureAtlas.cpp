#include "Engine/Renderer/TextureAtlas.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Engine
{
    namespace
    {
        constexpr int s_Channels = 4;

        int GetFaceIndex(int axis, bool isPositive)
        {
            switch (axis)
            {
            case 0:
                return isPositive ? 0 : 1;
            case 1:
                return isPositive ? 2 : 3;
            default:
                return isPositive ? 4 : 5;
            }
        }
    }

    TextureAtlas::~TextureAtlas()
    {
        // Release the OpenGL texture if it was allocated.
        if (m_TextureId != 0)
        {
            glDeleteTextures(1, &m_TextureId);
            m_TextureId = 0;
        }
    }

    bool TextureAtlas::BuildFromDefinitions(const std::string& textureDirectory, const std::vector<BlockTextureDefinition>& definitions, int tileSize)
    {
        // Clear any previous atlas state so rebuilding is safe.
        m_TextureOrigins.clear();
        m_BlockTable.clear();
        m_PixelData.clear();
        m_AtlasSize = 0;

        if (!LoadTextures(textureDirectory, definitions, tileSize))
        {
            return false;
        }

        UploadAtlas();

        // Populate the block lookup table with UV rectangles for each face.
        for (const BlockTextureDefinition& it_Definition : definitions)
        {
            BlockRenderInfo l_Info = {};
            l_Info.IsOpaque = it_Definition.IsOpaque;
            l_Info.MaterialFlags = it_Definition.MaterialFlags;

            for (int l_Axis = 0; l_Axis < 3; ++l_Axis)
            {
                glm::vec2 l_Min = {};
                glm::vec2 l_Max = {};
                const int l_PositiveIndex = GetFaceIndex(l_Axis, true);
                const int l_NegativeIndex = GetFaceIndex(l_Axis, false);

                auto it_Positive = m_TextureOrigins.find(it_Definition.FaceTextures[l_PositiveIndex]);
                auto it_Negative = m_TextureOrigins.find(it_Definition.FaceTextures[l_NegativeIndex]);

                if (it_Positive != m_TextureOrigins.end())
                {
                    l_Min = glm::vec2(it_Positive->second.x / static_cast<float>(m_AtlasSize), it_Positive->second.y / static_cast<float>(m_AtlasSize));
                    l_Max = glm::vec2((it_Positive->second.x + static_cast<float>(tileSize)) / static_cast<float>(m_AtlasSize), (it_Positive->second.y + static_cast<float>(tileSize)) / static_cast<float>(m_AtlasSize));
                    l_Info.AtlasMins[l_PositiveIndex] = l_Min;
                    l_Info.AtlasMaxs[l_PositiveIndex] = l_Max;
                }

                if (it_Negative != m_TextureOrigins.end())
                {
                    l_Min = glm::vec2(it_Negative->second.x / static_cast<float>(m_AtlasSize), it_Negative->second.y / static_cast<float>(m_AtlasSize));
                    l_Max = glm::vec2((it_Negative->second.x + static_cast<float>(tileSize)) / static_cast<float>(m_AtlasSize), (it_Negative->second.y + static_cast<float>(tileSize)) / static_cast<float>(m_AtlasSize));
                    l_Info.AtlasMins[l_NegativeIndex] = l_Min;
                    l_Info.AtlasMaxs[l_NegativeIndex] = l_Max;
                }
            }

            m_BlockTable[it_Definition.Block] = l_Info;
        }

        return true;
    }

    bool TextureAtlas::LoadTextures(const std::string& textureDirectory, const std::vector<BlockTextureDefinition>& definitions, int tileSize)
    {
        // Cache unique textures so repeated face names are only loaded once.
        std::unordered_map<std::string, LoadedTexture> l_LoadedTextures = {};

        for (const BlockTextureDefinition& it_Definition : definitions)
        {
            for (const std::string& it_TextureName : it_Definition.FaceTextures)
            {
                if (it_TextureName.empty() || l_LoadedTextures.contains(it_TextureName))
                {
                    continue;
                }

                std::filesystem::path l_Path = std::filesystem::path(textureDirectory) / (it_TextureName + ".png");
                LoadedTexture l_Texture = {};

                int l_Width = 0;
                int l_Height = 0;
                int l_Channels = 0;
                unsigned char* l_Data = stbi_load(l_Path.string().c_str(), &l_Width, &l_Height, &l_Channels, s_Channels);

                if (l_Data == nullptr)
                {
                    std::cout << "Failed to load texture: " << l_Path.string() << std::endl;
                    continue;
                }

                if (l_Width != tileSize || l_Height != tileSize)
                {
                    std::cout << "Texture " << l_Path.string() << " does not match tile size " << tileSize << std::endl;
                    stbi_image_free(l_Data);
                    continue;
                }

                l_Texture.Width = l_Width;
                l_Texture.Height = l_Height;
                l_Texture.Pixels.assign(l_Data, l_Data + l_Width * l_Height * s_Channels);
                stbi_image_free(l_Data);

                l_LoadedTextures[it_TextureName] = l_Texture;
            }
        }

        if (l_LoadedTextures.empty())
        {
            std::cout << "Texture atlas definition did not reference any valid textures." << std::endl;
            return false;
        }

        PackAtlas(l_LoadedTextures, tileSize);

        return true;
    }

    void TextureAtlas::PackAtlas(const std::unordered_map<std::string, LoadedTexture>& textures, int tileSize)
    {
        const int l_TextureCount = static_cast<int>(textures.size());
        const int l_TilesPerRow = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(l_TextureCount))));

        // Build a square atlas; rounding up keeps packing straightforward.
        m_AtlasSize = l_TilesPerRow * tileSize;
        m_PixelData.assign(static_cast<std::size_t>(m_AtlasSize * m_AtlasSize * s_Channels), 0);
        m_TextureOrigins.clear();

        int l_Index = 0;
        for (const auto& it_Texture : textures)
        {
            int l_X = (l_Index % l_TilesPerRow) * tileSize;
            int l_Y = (l_Index / l_TilesPerRow) * tileSize;
            const LoadedTexture& l_Loaded = it_Texture.second;

            for (int row = 0; row < tileSize; ++row)
            {
                int l_Destination = ((l_Y + row) * m_AtlasSize + l_X) * s_Channels;
                int l_Source = row * tileSize * s_Channels;
                std::copy_n(l_Loaded.Pixels.data() + l_Source, tileSize * s_Channels, m_PixelData.begin() + l_Destination);
            }

            m_TextureOrigins[it_Texture.first] = glm::vec2(static_cast<float>(l_X), static_cast<float>(l_Y));
            ++l_Index;
        }
    }

    void TextureAtlas::UploadAtlas()
    {
        if (m_TextureId == 0)
        {
            glGenTextures(1, &m_TextureId);
        }

        glBindTexture(GL_TEXTURE_2D, m_TextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_AtlasSize, m_AtlasSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_PixelData.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}