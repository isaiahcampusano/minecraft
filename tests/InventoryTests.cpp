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
  const std::size_t expectedCatalog=(BLOCK_TYPE_COUNT-1)+static_cast<std::size_t>(MaterialType::COUNT)+static_cast<std::size_t>(FoodType::COUNT);if(creativeCatalog().size()!=expectedCatalog||creativeCatalog().back().kind!=ItemKind::FOOD||creativeCatalog().back().foodType!=FoodType::COOKED_MUTTON)return fail("creative catalog does not include all registered food items");
  if(itemName(ItemStack::tool(ToolKind::PICKAXE,ToolTier::WOOD,60))!="WOOD PICKAXE")return fail("tool item name did not include tier and kind");
  if(itemName(creativeCatalog().front())!=blockName(creativeCatalog().front().blockType))return fail("creative catalog item name did not match the block registry");
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
  Inventory lootItems;if(!lootItems.add(ItemStack::food(FoodType::RAW_BEEF,3))||!lootItems.add(ItemStack::food(FoodType::RAW_PORKCHOP,2))||!lootItems.add(ItemStack::food(FoodType::RAW_MUTTON))||!lootItems.add(ItemStack::material(MaterialType::LEATHER,2))||!lootItems.add(ItemStack::material(MaterialType::WHITE_WOOL)))return fail("mob loot items were rejected by inventory normalization");
  Inventory fullFood;for(int i=0;i<Inventory::HOTBAR_SLOTS;++i)fullFood.setHotbarSlot(i,ItemStack::block(types[static_cast<std::size_t>(i)],64));for(int i=0;i<Inventory::BACKPACK_SLOTS;++i)fullFood.setBackpackSlot(i,ItemStack::block(types[static_cast<std::size_t>(i%types.size())],64));if(fullFood.add(ItemStack::food(FoodType::APPLE)))return fail("apple was retained by a full inventory");

  inventory.select(-10); if (inventory.selectedSlot() != 0) return fail("negative selection was not clamped");
  inventory.select(99); if (inventory.selectedSlot() != Inventory::HOTBAR_SLOTS - 1) return fail("high selection was not clamped");

  Inventory rightClick;rightClick.setHotbarSlot(0,ItemStack::block(BlockType::STONE,5));
  rightClick.rightClick(Inventory::Area::HOTBAR,0);
  if(rightClick.cursorStack().count!=3||rightClick.hotbarSlot(0).count!=2)return fail("right-click on an empty cursor did not pick up half, rounded up");
  rightClick.rightClick(Inventory::Area::HOTBAR,1);
  if(rightClick.hotbarSlot(1).count!=1||rightClick.hotbarSlot(1).blockType!=BlockType::STONE||rightClick.cursorStack().count!=2)return fail("right-click with a held stack did not place a single item into an empty slot");
  rightClick.rightClick(Inventory::Area::HOTBAR,1);
  if(rightClick.hotbarSlot(1).count!=2||rightClick.cursorStack().count!=1)return fail("right-click did not add a single item onto a matching stack");

  Inventory leftDrag;leftDrag.setCursorStack(ItemStack::block(BlockType::DIRT,6));
  leftDrag.beginDrag(false);
  leftDrag.dragOver(Inventory::Area::HOTBAR,0);leftDrag.dragOver(Inventory::Area::HOTBAR,1);leftDrag.dragOver(Inventory::Area::HOTBAR,2);
  leftDrag.dragOver(Inventory::Area::HOTBAR,0);
  if(leftDrag.dragSlotCount()!=3)return fail("left-drag recorded a slot more than once");
  leftDrag.endDrag();
  if(leftDrag.hotbarSlot(0).count!=2||leftDrag.hotbarSlot(1).count!=2||leftDrag.hotbarSlot(2).count!=2||!leftDrag.cursorStack().empty())return fail("left-drag did not evenly split the held stack across the dragged slots");
  if(leftDrag.isDragging()||leftDrag.dragSlotCount()!=0)return fail("endDrag did not clear drag state");

  Inventory scarceDrag;scarceDrag.setCursorStack(ItemStack::block(BlockType::SAND,2));
  scarceDrag.beginDrag(false);for(int i=0;i<5;++i)scarceDrag.dragOver(Inventory::Area::HOTBAR,i);scarceDrag.endDrag();
  int filled=0;for(int i=0;i<5;++i)if(scarceDrag.hotbarSlot(i).count==1)++filled;
  if(filled!=2||!scarceDrag.cursorStack().empty())return fail("left-drag with fewer items than slots did not give one item to as many slots as it could afford");

  Inventory rightDrag;rightDrag.setCursorStack(ItemStack::block(BlockType::GRAVEL,5));
  rightDrag.beginDrag(true);
  rightDrag.dragOver(Inventory::Area::BACKPACK,0);rightDrag.dragOver(Inventory::Area::BACKPACK,1);rightDrag.dragOver(Inventory::Area::BACKPACK,2);
  if(rightDrag.backpackSlot(0).count!=1||rightDrag.backpackSlot(1).count!=1||rightDrag.backpackSlot(2).count!=1||rightDrag.cursorStack().count!=2)return fail("right-drag did not place one item per slot as it was painted");
  rightDrag.endDrag();
  if(rightDrag.cursorStack().count!=2||rightDrag.backpackSlot(0).count!=1)return fail("endDrag altered an already-applied right-drag");

  Inventory abandonedDrag;abandonedDrag.setCursorStack(ItemStack::block(BlockType::GLASS,4));
  abandonedDrag.beginDrag(false);abandonedDrag.dragOver(Inventory::Area::HOTBAR,0);
  if(abandonedDrag.dragSlotCount()!=1)return fail("a single-slot drag should record exactly one slot");
  abandonedDrag.endDrag();
  if(abandonedDrag.hotbarSlot(0).count!=0||abandonedDrag.cursorStack().count!=4)return fail("a drag that only touched one slot should not distribute");

  Inventory quickMove;
  quickMove.setHotbarSlot(0,ItemStack::block(BlockType::STONE,64));
  if(!quickMove.quickMove(Inventory::Area::HOTBAR,0)||!quickMove.hotbarSlot(0).empty()||quickMove.backpackSlot(0).count!=64)return fail("quick move did not route hotbar items to backpack");
  if(!quickMove.quickMove(Inventory::Area::BACKPACK,0)||quickMove.hotbarSlot(0).count!=64||!quickMove.backpackSlot(0).empty())return fail("quick move did not route backpack items back to hotbar");

  return 0;
}
