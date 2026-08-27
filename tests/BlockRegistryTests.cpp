#include "world/BlockRegistry.h"
#include <array>
#include <iostream>
#include <string>

namespace { int fail(const char* message){std::cerr<<message<<'\n';return 1;} }

int main(){
  if(BLOCK_COLORS.size()!=BLOCK_TYPE_COUNT||BLOCK_NAMES.size()!=BLOCK_TYPE_COUNT)return fail("registry tables do not match BlockType count");
  std::array<bool,BLOCK_TYPE_COUNT> seen{};bool foundLeaves=false,foundPlanks=false,foundTable=false;
  for(BlockType type:allPlaceableBlocks()){
    const auto index=static_cast<std::size_t>(type);
    if(type==BlockType::AIR||index>=seen.size()||seen[index])return fail("placeable registry contains an invalid or duplicate block");
    seen[index]=true;foundLeaves|=type==BlockType::LEAVES;foundPlanks|=type==BlockType::PLANKS;foundTable|=type==BlockType::CRAFTING_TABLE;
  }
  if(!foundLeaves||!seen[static_cast<std::size_t>(BlockType::LEAVES)])return fail("LEAVES was missing from placeable blocks");
  if(!foundPlanks||!foundTable)return fail("crafting blocks were missing from placeable blocks");
  if(std::string(blockName(BlockType::LEAVES))!="LEAVES")return fail("LEAVES name was not registered");
  const BlockColor leaves=blockColor(BlockType::LEAVES);
  if(leaves.r!=51||leaves.g!=153||leaves.b!=26)return fail("LEAVES color was not the oak-green registry swatch");
  for(BlockType type:allPlaceableBlocks())if(type!=BlockType::LEAVES){const BlockColor other=blockColor(type);if(other.r==leaves.r&&other.g==leaves.g&&other.b==leaves.b)return fail("LEAVES color duplicated another placeable block");}
  return 0;
}
