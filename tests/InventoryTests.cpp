#include "player/Inventory.h"
#include "world/BlockRegistry.h"
#include "player/CreativeCatalog.h"
#include <array>
#include <iostream>

namespace { int fail(const char* message) { std::cerr << message << '\n'; return 1; } }

int main() {
  Inventory inventory;
  if (!inventory.add(BlockType::STONE) || inventory.hotbarSlot(0).blockType != BlockType::STONE || inventory.hotbarSlot(0).count != 1) return fail("add did not fill the first hotbar slot");
  if (!inventory.add(BlockType::STONE) || inventory.hotbarSlot(0).count != 2 || inventory.hotbarSlot(1).count != 0) return fail("add did not stack a matching item");
  if (!inventory.consumeSelected() || inventory.hotbarSlot(0).count != 1) return fail("consume did not decrement the selected stack");
  if (!inventory.consumeSelected() || inventory.hotbarSlot(0).count != 0 || inventory.hotbarSlot(0).blockType != BlockType::AIR) return fail("consume did not clear an exhausted stack");
  if (inventory.consumeSelected()) return fail("consume succeeded on an empty slot");

  const std::array<BlockType,Inventory::HOTBAR_SLOTS> types={BlockType::GRASS,BlockType::DIRT,BlockType::BEDROCK,BlockType::STONE,BlockType::COBBLESTONE,BlockType::OAK_LOG,BlockType::SAND,BlockType::GRAVEL,BlockType::GLASS};
  Inventory overflow;
  for(int i=0;i<Inventory::HOTBAR_SLOTS;++i){if(!overflow.giveCreative(types[static_cast<std::size_t>(i)]))return fail("could not create full stack");overflow.swapHotbar(i);}
  if(!overflow.add(BlockType::GRASS)||overflow.backpackSlot(0).blockType!=BlockType::GRASS)return fail("overflow did not reach backpack");
  overflow.giveCreative(BlockType::GRASS);overflow.swapBackpack(0);
  for(int i=1;i<Inventory::BACKPACK_SLOTS;++i){BlockType type=types[static_cast<std::size_t>(i%types.size())];overflow.giveCreative(type);overflow.swapBackpack(i);}
  if(overflow.add(BlockType::GRASS))return fail("add succeeded when hotbar and backpack were full");

  Inventory swapping;
  swapping.giveCreative(BlockType::GLASS);swapping.swapBackpack(4);
  if(swapping.cursorStack().count!=0||swapping.backpackSlot(4).blockType!=BlockType::GLASS)return fail("empty backpack slot did not receive cursor stack");
  swapping.swapHotbar(2);
  if(swapping.cursorStack().count!=0||swapping.hotbarSlot(2).count!=0)return fail("empty-to-empty swap changed stacks");
  swapping.swapBackpack(4);swapping.add(BlockType::STONE);swapping.swapHotbar(0);
  if(swapping.cursorStack().blockType!=BlockType::STONE||swapping.hotbarSlot(0).blockType!=BlockType::GLASS)return fail("hotbar swap did not exchange stacks");

  std::array<bool,BLOCK_TYPE_COUNT> seen{};std::size_t creativeCount=0;
  for(BlockType type:allPlaceableBlocks()){auto index=static_cast<std::size_t>(type);if(type==BlockType::AIR||index>=seen.size()||seen[index])return fail("creative block range is invalid");seen[index]=true;++creativeCount;}
  if(creativeCount!=BLOCK_TYPE_COUNT-1)return fail("creative range does not cover every non-air block");
  if(creativeCatalog().size()!=BLOCK_TYPE_COUNT||creativeCatalog().back().kind!=ItemKind::FOOD||creativeCatalog().back().foodType!=FoodType::APPLE)return fail("creative catalog does not include apples");
  Inventory creative;
  if(!creative.giveCreative(BlockType::LEAVES)||creative.cursorStack().blockType!=BlockType::LEAVES||creative.cursorStack().count!=Inventory::CREATIVE_STACK_SIZE)return fail("creative give did not create a full LEAVES stack");
  creative.swapBackpack(0);
  if(!creative.giveCreative(BlockType::LEAVES)||creative.cursorStack().count!=Inventory::CREATIVE_STACK_SIZE||creative.backpackSlot(0).blockType!=BlockType::LEAVES||creative.backpackSlot(0).count!=Inventory::CREATIVE_STACK_SIZE)return fail("creative source was depleted");

  Inventory crafting;crafting.setCursorStack(ItemStack::block(BlockType::PLANKS,4));for(int i=0;i<4;++i)crafting.swapCraft(false,i);if(!crafting.cursorStack().empty())return fail("crafting grid did not distribute a stack one item at a time");if(crafting.craftingOutput(false).blockType!=BlockType::CRAFTING_TABLE||!crafting.craftOutput(false)||crafting.cursorStack().blockType!=BlockType::CRAFTING_TABLE)return fail("personal grid could not produce a crafting table through inventory interaction");for(int i=0;i<4;++i)if(!crafting.craftSlot(false,i).empty())return fail("crafted ingredients were not consumed");

  Inventory restoring;restoring.setCursorStack(ItemStack::block(BlockType::LEAVES,99));restoring.setHotbarSlot(8,ItemStack::block(BlockType::GLASS,12));restoring.setBackpackSlot(26,ItemStack::block(BlockType::COBBLESTONE,41));
  if(restoring.cursorStack().blockType!=BlockType::LEAVES||restoring.cursorStack().count!=Inventory::MAX_STACK_SIZE)return fail("restored cursor stack was not normalized");
  if(restoring.hotbarSlot(8).blockType!=BlockType::GLASS||restoring.hotbarSlot(8).count!=12||restoring.backpackSlot(26).blockType!=BlockType::COBBLESTONE||restoring.backpackSlot(26).count!=41)return fail("inventory slot setters did not restore stacks");
  restoring.setCursorStack(ItemStack::block(BlockType::AIR,4));if(restoring.cursorStack().blockType!=BlockType::AIR||restoring.cursorStack().count!=0)return fail("invalid restored stack was not cleared");
  Inventory food;food.setHotbarSlot(0,ItemStack::food(FoodType::APPLE,63));if(!food.add(ItemStack::food(FoodType::APPLE,2))||food.hotbarSlot(0).count!=64||food.hotbarSlot(1).kind!=ItemKind::FOOD||food.hotbarSlot(1).count!=1)return fail("apple normalization or stacking failed");if(!food.consumeSelectedFood(FoodType::APPLE)||food.hotbarSlot(0).count!=63)return fail("selected apple was not consumed");food.setCursorStack(ItemStack::food(FoodType::APPLE,3));food.swapCraft(false,0);food.setCursorStack(ItemStack::food(FoodType::APPLE,2));food.swapCraft(true,0);food.setBackpackSlot(0,ItemStack::food(FoodType::APPLE,4));food.clear();if(!food.cursorStack().empty()||!food.hotbarSlot(0).empty()||!food.backpackSlot(0).empty()||!food.craftSlot(false,0).empty()||!food.craftSlot(true,0).empty()||food.selectedSlot()!=0)return fail("death inventory clearance was incomplete");
  Inventory fullFood;for(int i=0;i<Inventory::HOTBAR_SLOTS;++i)fullFood.setHotbarSlot(i,ItemStack::block(types[static_cast<std::size_t>(i)],64));for(int i=0;i<Inventory::BACKPACK_SLOTS;++i)fullFood.setBackpackSlot(i,ItemStack::block(types[static_cast<std::size_t>(i%types.size())],64));if(fullFood.add(ItemStack::food(FoodType::APPLE)))return fail("apple was retained by a full inventory");

  inventory.select(-10); if (inventory.selectedSlot() != 0) return fail("negative selection was not clamped");
  inventory.select(99); if (inventory.selectedSlot() != Inventory::HOTBAR_SLOTS - 1) return fail("high selection was not clamped");
  return 0;
}
