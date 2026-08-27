#include "BlockProperties.h"
#include <array>

namespace {
constexpr std::array<BlockProperties,BLOCK_TYPE_COUNT> PROPERTIES={{
  {0.f,false},   // AIR
  {1.2f,true},   // GRASS
  {.5f,true},    // DIRT
  {-1.f,false},  // BEDROCK
  {4.f,true},    // STONE
  {4.f,true},    // COBBLESTONE: stone and cobblestone share material hardness.
  {2.f,true},    // OAK_LOG
  {.6f,true},    // SAND
  {.2f,true},    // GRAVEL
  {.6f,true},    // GLASS
  {.2f,true}     // LEAVES
}};
constexpr BlockProperties INVALID{0.f,false};
}

const BlockProperties& getBlockProperties(BlockType type){
  auto index=static_cast<std::size_t>(type);
  return index<PROPERTIES.size()?PROPERTIES[index]:INVALID;
}
