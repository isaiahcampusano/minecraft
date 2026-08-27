#include "core/GameState.h"
#include "player/Inventory.h"
#include "world/World.h"
#include <iostream>

namespace { int fail(const char* message){std::cerr<<message<<'\n';return 1;} }

int main(){
  Inventory sourceInventory;sourceInventory.select(5);sourceInventory.setCursorStack(ItemStack::material(MaterialType::STICK,64));sourceInventory.setHotbarSlot(5,ItemStack::tool(ToolKind::PICKAXE,ToolTier::STONE,73));sourceInventory.setBackpackSlot(26,ItemStack::block(BlockType::COBBLESTONE,41));
  World sourceWorld;if(!sourceWorld.setBlock(17,8,17,BlockType::LEAVES)||!sourceWorld.setBlock(18,6,18,BlockType::AIR)||!sourceWorld.setBlock(900,8,900,BlockType::GLASS))return fail("could not create source edits");
  const SaveData data=GameState::capture(sourceInventory,sourceWorld);if(data.edits.size()!=3)return fail("capture did not include every sparse world edit");
  Inventory restoredInventory;World restoredWorld;GameState::apply(data,restoredInventory,restoredWorld);
  if(restoredInventory.selectedSlot()!=5||restoredInventory.cursorStack().kind!=ItemKind::MATERIAL||restoredInventory.cursorStack().materialType!=MaterialType::STICK||restoredInventory.cursorStack().count!=64)return fail("cursor or selection was not restored");
  if(restoredInventory.hotbarSlot(5).kind!=ItemKind::TOOL||restoredInventory.hotbarSlot(5).toolTier!=ToolTier::STONE||restoredInventory.hotbarSlot(5).durability!=73)return fail("tool was not restored");
  if(restoredInventory.backpackSlot(26).blockType!=BlockType::COBBLESTONE||restoredInventory.backpackSlot(26).count!=41)return fail("backpack was not restored");
  restoredWorld.loadChunk(1,1);restoredWorld.loadChunk(56,56);
  if(restoredWorld.getBlock(17,8,17)!=BlockType::LEAVES||restoredWorld.getBlock(18,6,18)!=BlockType::AIR)return fail("nearby placed/broken blocks were not restored");
  if(restoredWorld.getBlock(900,8,900)!=BlockType::GLASS)return fail("distant edit was not restored when its chunk loaded");
  if(restoredWorld.getEditEntries().size()!=3)return fail("restored edit overlay was incomplete");
  return 0;
}
