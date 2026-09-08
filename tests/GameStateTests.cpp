#include "core/GameState.h"
#include "player/Inventory.h"
#include "player/Player.h"
#include "world/World.h"
#include "world/PassiveMobSystem.h"
#include <iostream>

namespace { int fail(const char* message){std::cerr<<message<<'\n';return 1;} }

int main(){
  Inventory sourceInventory;sourceInventory.select(5);sourceInventory.setCursorStack(ItemStack::material(MaterialType::STICK,64));sourceInventory.setHotbarSlot(5,ItemStack::tool(ToolKind::PICKAXE,ToolTier::STONE,73));sourceInventory.setBackpackSlot(26,ItemStack::block(BlockType::COBBLESTONE,41));
  World sourceWorld;if(!sourceWorld.setBlock(17,8,17,BlockType::LEAVES)||!sourceWorld.setBlock(18,6,18,BlockType::AIR)||!sourceWorld.setBlock(900,8,900,BlockType::GLASS))return fail("could not create source edits");
  Player sourcePlayer;sourcePlayer.survival.restore(14,17,3.5f,2.25f);PassiveMobSystem sourceMobs;sourceMobs.addMob(MobType::SHEEP,{510.5f,7.f,510.5f},true).health=5;const SaveData data=GameState::capture(sourceInventory,sourceWorld,sourcePlayer,sourceMobs);if(data.edits.size()!=3||data.mobs.size()!=1)return fail("capture did not include world edits and mobs");
  Inventory restoredInventory;World restoredWorld;Player restoredPlayer;PassiveMobSystem restoredMobs;GameState::apply(data,restoredInventory,restoredWorld,restoredPlayer,restoredMobs);
  if(restoredInventory.selectedSlot()!=5||restoredInventory.cursorStack().kind!=ItemKind::MATERIAL||restoredInventory.cursorStack().materialType!=MaterialType::STICK||restoredInventory.cursorStack().count!=64)return fail("cursor or selection was not restored");
  if(restoredInventory.hotbarSlot(5).kind!=ItemKind::TOOL||restoredInventory.hotbarSlot(5).toolTier!=ToolTier::STONE||restoredInventory.hotbarSlot(5).durability!=73)return fail("tool was not restored");
  if(restoredInventory.backpackSlot(26).blockType!=BlockType::COBBLESTONE||restoredInventory.backpackSlot(26).count!=41)return fail("backpack was not restored");
  if(restoredPlayer.survival.health()!=14||restoredPlayer.survival.hunger()!=17||restoredPlayer.survival.saturation()!=3.5f||restoredPlayer.survival.exhaustion()!=2.25f)return fail("survival state was not restored");
  if(restoredMobs.mobs().size()!=1||restoredMobs.mobs()[0].type!=MobType::SHEEP||restoredMobs.mobs()[0].health!=5||!restoredMobs.mobs()[0].isBaby()||restoredMobs.mobs()[0].state!=MobAIState::IDLE)return fail("mob state was not restored with transient AI reset");
  restoredWorld.loadChunk(1,1);restoredWorld.loadChunk(56,56);
  if(restoredWorld.getBlock(17,8,17)!=BlockType::LEAVES||restoredWorld.getBlock(18,6,18)!=BlockType::AIR)return fail("nearby placed/broken blocks were not restored");
  if(restoredWorld.getBlock(900,8,900)!=BlockType::GLASS)return fail("distant edit was not restored when its chunk loaded");
  if(restoredWorld.getEditEntries().size()!=3)return fail("restored edit overlay was incomplete");
  return 0;
}
