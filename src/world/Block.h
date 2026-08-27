#pragma once
#include <cstddef>
#include <cstdint>

enum class BlockType : std::uint8_t {
  AIR = 0, GRASS, DIRT, BEDROCK, STONE, COBBLESTONE, OAK_LOG, SAND, GRAVEL, GLASS, LEAVES, PLANKS, CRAFTING_TABLE, COUNT
};
inline constexpr std::size_t BLOCK_TYPE_COUNT=static_cast<std::size_t>(BlockType::COUNT);
inline bool isSolid(BlockType type) { return type != BlockType::AIR && type != BlockType::COUNT; }

