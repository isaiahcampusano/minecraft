#include "BlockProperties.h"
#include <array>

namespace {
constexpr std::array<BlockProperties,BLOCK_TYPE_COUNT> PROPERTIES={{
  {0.f,false,std::nullopt,std::nullopt},
  {1.2f,true,ToolKind::SHOVEL,std::nullopt},
  {.5f,true,ToolKind::SHOVEL,std::nullopt},
  {-1.f,false,std::nullopt,std::nullopt},
  {4.f,true,ToolKind::PICKAXE,ToolTier::WOOD},
  {4.f,true,ToolKind::PICKAXE,ToolTier::WOOD},
  {2.f,true,ToolKind::AXE,std::nullopt},
  {.6f,true,ToolKind::SHOVEL,std::nullopt},
  {.2f,true,ToolKind::SHOVEL,std::nullopt},
  {.6f,true,std::nullopt,std::nullopt},
  {.2f,true,std::nullopt,std::nullopt},
  {2.f,true,ToolKind::AXE,std::nullopt},
  {2.5f,true,ToolKind::PICKAXE,ToolTier::WOOD}
}};
constexpr BlockProperties INVALID{0.f,false,std::nullopt,std::nullopt};
}

BlockType miningDrop(BlockType mined){return mined==BlockType::STONE?BlockType::COBBLESTONE:mined;}
bool toolMatches(const ItemStack& held,const BlockProperties& properties){return properties.effectiveTool&&held.kind==ItemKind::TOOL&&!held.empty()&&held.toolKind==*properties.effectiveTool;}
float miningSpeedMultiplier(const ItemStack& held,const BlockProperties& properties){if(!toolMatches(held,properties))return 1.f;return held.toolTier==ToolTier::STONE?3.f:2.f;}
bool canDropBlock(const ItemStack& held,const BlockProperties& properties){return !properties.minTierToDrop||(toolMatches(held,properties)&&static_cast<std::uint8_t>(held.toolTier)>=static_cast<std::uint8_t>(*properties.minTierToDrop));}

const BlockProperties& getBlockProperties(BlockType type){
  auto index=static_cast<std::size_t>(type);
  return index<PROPERTIES.size()?PROPERTIES[index]:INVALID;
}
