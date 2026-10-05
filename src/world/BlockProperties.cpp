#include "BlockProperties.h"
#include "../player/ToolRegistry.h"
#include <array>

namespace {
constexpr std::array<BlockProperties,BLOCK_TYPE_COUNT> PROPERTIES={{
  {0.f,false,std::nullopt,std::nullopt,true,1.f,false},
  {1.2f,true,ToolKind::SHOVEL,std::nullopt,true,1.f,false},
  {.5f,true,ToolKind::SHOVEL,std::nullopt,true,1.2f,false},
  {-1.f,false,std::nullopt,std::nullopt,false,1.f,false},
  {4.f,true,ToolKind::PICKAXE,ToolTier::WOOD,true,1.f,false},
  {4.f,true,ToolKind::PICKAXE,ToolTier::WOOD,true,1.2f,false},
  {2.f,true,ToolKind::AXE,std::nullopt,true,1.3f,false},
  {.6f,true,ToolKind::SHOVEL,std::nullopt,true,1.6f,false},
  {.2f,true,ToolKind::SHOVEL,std::nullopt,true,1.5f,false},
  {.6f,true,std::nullopt,std::nullopt,false,1.f,false},
  {.2f,true,std::nullopt,std::nullopt,true,1.25f,false},
  {2.f,true,ToolKind::AXE,std::nullopt,true,1.3f,false},
  {2.5f,true,ToolKind::PICKAXE,ToolTier::WOOD,true,1.f,false},
  {0.f,false,std::nullopt,std::nullopt,true,1.f,false},
  {3.5f,true,ToolKind::PICKAXE,ToolTier::WOOD,true,1.f,false},
  {3.f,true,ToolKind::PICKAXE,std::nullopt,true,1.f,false},
  {3.f,true,ToolKind::PICKAXE,ToolTier::STONE,true,1.f,false}
}};
constexpr BlockProperties INVALID{0.f,false,std::nullopt,std::nullopt,false,1.f,false};
}

ItemStack miningDrop(BlockType mined){
  if(mined==BlockType::STONE)return ItemStack::block(BlockType::COBBLESTONE);
  if(mined==BlockType::COAL_ORE)return ItemStack::material(MaterialType::COAL);
  return ItemStack::block(mined);
}
bool toolMatches(const ItemStack& held,const BlockProperties& properties){return properties.effectiveTool&&held.kind==ItemKind::TOOL&&!held.empty()&&held.toolKind==*properties.effectiveTool;}
float miningSpeedMultiplier(const ItemStack& held,const BlockProperties& properties){if(!toolMatches(held,properties))return 1.f;const auto tier=static_cast<std::size_t>(held.toolTier);return tier<TOOL_MINING_SPEED.size()?TOOL_MINING_SPEED[tier]:1.f;}
bool canDropBlock(const ItemStack& held,const BlockProperties& properties){return !properties.minTierToDrop||(toolMatches(held,properties)&&static_cast<std::uint8_t>(held.toolTier)>=static_cast<std::uint8_t>(*properties.minTierToDrop));}
bool canDropBlock(BlockType mined,const ItemStack& held){return mined!=BlockType::LEAVES&&canDropBlock(held,getBlockProperties(mined));}

const BlockProperties& getBlockProperties(BlockType type){
  auto index=static_cast<std::size_t>(type);
  return index<PROPERTIES.size()?PROPERTIES[index]:INVALID;
}
