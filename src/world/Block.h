#pragma once
#include <cstdint>

enum class BlockType : std::uint8_t {
  AIR = 0, GRASS, DIRT, BEDROCK, STONE, COBBLESTONE, PLANKS, SAND, GLASS
};
inline bool isSolid(BlockType type) { return type != BlockType::AIR; }

