#include "player/Inventory.h"
#include "player/Player.h"
#include "player/ToolRegistry.h"
#include "world/BlockProperties.h"
#include <cmath>
#include <iostream>

namespace {int fail(const char* message){std::cerr<<message<<'\n';return 1;}bool near(float a,float b){return std::abs(a-b)<.001f;}}

int main(){
  const auto& stone=getBlockProperties(BlockType::STONE);ItemStack hand;
  if(canDropBlock(hand,stone))return fail("hand mining stone was allowed to drop an item");
  if(canDropBlock(BlockType::STONE,hand))return fail("mining flow allowed a hand-mined stone drop");
  if(canDropBlock(BlockType::LEAVES,hand))return fail("leaves dropped themselves without shears or silk touch");
  if(!canDropBlock(BlockType::DIRT,hand))return fail("ordinary hand-mined blocks stopped dropping themselves");
  const ItemStack woodPick=ItemStack::tool(ToolKind::PICKAXE,ToolTier::WOOD,maxToolDurability(ToolTier::WOOD));
  const ItemStack stonePick=ItemStack::tool(ToolKind::PICKAXE,ToolTier::STONE,maxToolDurability(ToolTier::STONE));
  if(canDropBlock(BlockType::LEAVES,woodPick))return fail("leaves dropped themselves with an unrelated tool");
  if(!canDropBlock(BlockType::STONE,woodPick)||miningDrop(BlockType::STONE)!=BlockType::COBBLESTONE)return fail("wood pickaxe did not unlock the cobblestone drop");
  Player player;player.setMiningTarget({1,2,3});for(int i=0;i<120;++i)player.advanceMining(BlockType::STONE,Player::MINING_FIXED_STEP,hand);if(!near(player.blockBreakProgress,.25f))return fail("hand mining speed changed");
  player.clearMiningTarget();player.setMiningTarget({1,2,3});for(int i=0;i<120;++i)player.advanceMining(BlockType::STONE,Player::MINING_FIXED_STEP,woodPick);if(!near(player.blockBreakProgress,.5f))return fail("wood pickaxe was not twice hand speed");
  player.clearMiningTarget();player.setMiningTarget({1,2,3});for(int i=0;i<120;++i)player.advanceMining(BlockType::STONE,Player::MINING_FIXED_STEP,stonePick);if(!near(player.blockBreakProgress,.75f))return fail("stone pickaxe was not faster than wood");
  const ItemStack axe=ItemStack::tool(ToolKind::AXE,ToolTier::STONE,maxToolDurability(ToolTier::STONE));if(toolMatches(axe,stone)||!near(miningSpeedMultiplier(axe,stone),1.f))return fail("mismatched tool received a mining bonus");
  Inventory inventory;inventory.setHotbarSlot(0,woodPick);for(int i=1;i<maxToolDurability(ToolTier::WOOD);++i)if(!inventory.damageSelectedTool()||inventory.selectedStack().empty())return fail("wood tool broke before its documented durability");if(!inventory.damageSelectedTool()||!inventory.selectedStack().empty())return fail("wood tool did not clear at exactly zero durability");
  inventory.setHotbarSlot(0,axe);const int before=inventory.selectedStack().durability;if(toolMatches(inventory.selectedStack(),stone))inventory.damageSelectedTool();if(inventory.selectedStack().durability!=before)return fail("mismatched tool took category durability damage");
  if(toolAttackDamage(ToolKind::SWORD,ToolTier::WOOD)!=4||toolAttackDamage(ToolKind::SWORD,ToolTier::STONE)!=5)return fail("sword damage did not scale by tier");
  if(toolAttackDamage(ToolKind::PICKAXE,ToolTier::WOOD)!=2||toolAttackDamage(ToolKind::PICKAXE,ToolTier::STONE)!=3)return fail("pickaxe damage did not scale by tier");
  if(toolAttackDamage(ToolKind::HOE,ToolTier::WOOD)!=2)return fail("hoe damage table was not initialized");
  return 0;
}
