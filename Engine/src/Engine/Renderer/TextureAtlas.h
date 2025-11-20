#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <glad/glad.h>
#include <glm/vec2.hpp>

#include "Engine/World/ChunkMesher.h"

namespace Engine
{
    // Describes the textures that should be applied to a block type.
    struct BlockTextureDefinition
    {
        BlockType Block = BlockType::Air;
        bool IsOpaque = true;
        float MaterialFlags = 0.0f;
        std::array<std::string, 6> FaceTextures = {};
    };

    // Tracks atlas UV rectangles for every block face and owns the OpenGL texture object.
    class TextureAtlas
    {
    public:
        TextureAtlas() = default;
        ~TextureAtlas();

        bool BuildFromDefinitions(const std::string& textureDirectory, const std::vector<BlockTextureDefinition>& definitions, int tileSize = 16);

        GLuint GetTextureId() const { return m_TextureId; }
        const std::unordered_map<BlockType, BlockRenderInfo>& GetBlockTable() const { return m_BlockTable; }

    private:
        struct LoadedTexture
        {
            int Width = 0;
            int Height = 0;
            std::vector<unsigned char> Pixels;
        };

        bool LoadTextures(const std::string& textureDirectory, const std::vector<BlockTextureDefinition>& definitions, int tileSize);
        void PackAtlas(const std::unordered_map<std::string, LoadedTexture>& textures, int tileSize);
        void UploadAtlas();

    private:
        GLuint m_TextureId = 0;
        int m_AtlasSize = 0;
        std::vector<unsigned char> m_PixelData;
        std::unordered_map<std::string, glm::vec2> m_TextureOrigins;
        std::unordered_map<BlockType, BlockRenderInfo> m_BlockTable;
    };
}