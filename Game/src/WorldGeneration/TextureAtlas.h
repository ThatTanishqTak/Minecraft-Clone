#pragma once

#include <array>
#include <string>
#include <unordered_map>
#include <memory>

#include <glm/glm.hpp>

#include "Engine/Renderer/Texture2D.h"
#include "Block.h"

// Handles loading a texture atlas from disk and producing per-face UVs for blocks.
class TextureAtlas
{
public:
    TextureAtlas() = default;

    bool Load(const std::string& filePath, const glm::ivec2& tileSize);

    void RegisterBlockFace(BlockId blockId, BlockFace face, const glm::ivec2& tileIndex);
    BlockFaceUV GetFaceUVs(BlockId blockId, BlockFace face) const;

    const Engine::Texture2D* GetTexture() const { return m_Texture.get(); }
    glm::ivec2 GetTileSize() const { return m_TileSize; }

private:
    glm::ivec2 m_TileSize{ 1, 1 };
    glm::ivec2 m_TextureSize{ 1, 1 };
    std::unique_ptr<Engine::Texture2D> m_Texture;
    std::unordered_map<BlockId, std::array<BlockFaceUV, static_cast<size_t>(BlockFace::Count)>> m_BlockFaceUVs;
};