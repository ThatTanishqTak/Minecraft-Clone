#include "TextureAtlas.h"

#include "Engine/Core/Log.h"

bool TextureAtlas::Load(const std::string& filePath, const glm::ivec2& tileSize)
{
    // Load the atlas texture with stb_image via the engine wrapper.
    m_Texture = std::make_unique<Engine::Texture2D>(filePath);
    if (m_Texture == nullptr || !m_Texture->IsValid())
    {
        GAME_ERROR("Failed to load texture atlas: {}", filePath);

        return false;
    }

    GAME_INFO("Texture atlas loaded: {} (tile size: {}x{})", filePath, tileSize.x, tileSize.y);

    m_TileSize = tileSize;
    m_TextureSize = glm::ivec2{ m_Texture->GetWidth(), m_Texture->GetHeight() };

    return true;
}

void TextureAtlas::RegisterBlockFace(BlockId blockID, BlockFace face, const glm::ivec2& tileIndex)
{
    // Precompute UVs for each face to avoid per-vertex texture math during meshing.
    const glm::vec2 l_Texel = 1.0f / glm::vec2(m_TextureSize);
    const glm::vec2 l_UVMin = (glm::vec2(tileIndex * m_TileSize) + glm::vec2(0.5f)) * l_Texel;
    const glm::vec2 l_UVMax = (glm::vec2((tileIndex + glm::ivec2{ 1 }) * m_TileSize) - glm::vec2(0.5f)) * l_Texel;

    BlockFaceUV l_FaceUV{};
    l_FaceUV.m_UV00 = l_UVMin;
    l_FaceUV.m_UV10 = glm::vec2{ l_UVMax.x, l_UVMin.y };
    l_FaceUV.m_UV11 = l_UVMax;
    l_FaceUV.m_UV01 = glm::vec2{ l_UVMin.x, l_UVMax.y };

    m_BlockFaceUVs[blockID][static_cast<size_t>(face)] = l_FaceUV;
}

BlockFaceUV TextureAtlas::GetFaceUVs(BlockId blockID, BlockFace face) const
{
    const auto l_FaceIt = m_BlockFaceUVs.find(blockID);
    if (l_FaceIt != m_BlockFaceUVs.end())
    {
        return l_FaceIt->second[static_cast<size_t>(face)];
    }

    return {};
}