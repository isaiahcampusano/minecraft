#include "BlockProperties.h"
#include <array>

namespace {
constexpr std::array<BlockProperties,10> PROPERTIES={{
  {0.f,false},   // AIR
  {1.2f,true},   // GRASS
  {.5f,true},    // DIRT
  {-1.f,false},  // BEDROCK
  {4.f,true},    // STONE
  {10.f,true},   // COBBLESTONE
  {2.f,true},    // OAK_LOG
  {.6f,true},    // SAND
  {.2f,true},    // GRAVEL
  {.6f,true}     // GLASS
}};
constexpr BlockProperties INVALID{0.f,false};
}

const BlockProperties& getBlockProperties(BlockType type){
  auto index=static_cast<std::size_t>(type);
  return index<PROPERTIES.size()?PROPERTIES[index]:INVALID;
}
