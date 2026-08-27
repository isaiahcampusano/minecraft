#include "player/Inventory.h"
#include "world/BlockRegistry.h"
#include <array>
#include <iostream>

namespace { int fail(const char* message) { std::cerr << message << '\n'; return 1; } }

int main() {
  Inventory inventory;
  if (!inventory.add(BlockType::STONE) || inventory.hotbarSlot(0).type != BlockType::STONE || inventory.hotbarSlot(0).count != 1) return fail("add did not fill the first hotbar slot");
  if (!inventory.add(BlockType::STONE) || inventory.hotbarSlot(0).count != 2 || inventory.hotbarSlot(1).count != 0) return fail("add did not stack a matching item");
  if (!inventory.consumeSelected() || inventory.hotbarSlot(0).count != 1) return fail("consume did not decrement the selected stack");
  if (!inventory.consumeSelected() || inventory.hotbarSlot(0).count != 0 || inventory.hotbarSlot(0).type != BlockType::AIR) return fail("consume did not clear an exhausted stack");
  if (inventory.consumeSelected()) return fail("consume succeeded on an empty slot");

  const std::array<BlockType,Inventory::HOTBAR_SLOTS> types={BlockType::GRASS,BlockType::DIRT,BlockType::BEDROCK,BlockType::STONE,BlockType::COBBLESTONE,BlockType::OAK_LOG,BlockType::SAND,BlockType::GRAVEL,BlockType::GLASS};
  Inventory overflow;
  for(int i=0;i<Inventory::HOTBAR_SLOTS;++i){if(!overflow.giveCreative(types[static_cast<std::size_t>(i)]))return fail("could not create full stack");overflow.swapHotbar(i);}
  if(!overflow.add(BlockType::GRASS)||overflow.backpackSlot(0).type!=BlockType::GRASS)return fail("overflow did not reach backpack");
  overflow.giveCreative(BlockType::GRASS);overflow.swapBackpack(0);
  for(int i=1;i<Inventory::BACKPACK_SLOTS;++i){BlockType type=types[static_cast<std::size_t>(i%types.size())];overflow.giveCreative(type);overflow.swapBackpack(i);}
  if(overflow.add(BlockType::GRASS))return fail("add succeeded when hotbar and backpack were full");

  Inventory swapping;
  swapping.giveCreative(BlockType::GLASS);swapping.swapBackpack(4);
  if(swapping.cursorStack().count!=0||swapping.backpackSlot(4).type!=BlockType::GLASS)return fail("empty backpack slot did not receive cursor stack");
  swapping.swapHotbar(2);
  if(swapping.cursorStack().count!=0||swapping.hotbarSlot(2).count!=0)return fail("empty-to-empty swap changed stacks");
  swapping.swapBackpack(4);swapping.add(BlockType::STONE);swapping.swapHotbar(0);
  if(swapping.cursorStack().type!=BlockType::STONE||swapping.hotbarSlot(0).type!=BlockType::GLASS)return fail("hotbar swap did not exchange stacks");

  std::array<bool,BLOCK_TYPE_COUNT> seen{};std::size_t creativeCount=0;
  for(BlockType type:allPlaceableBlocks()){auto index=static_cast<std::size_t>(type);if(type==BlockType::AIR||index>=seen.size()||seen[index])return fail("creative block range is invalid");seen[index]=true;++creativeCount;}
  if(creativeCount!=BLOCK_TYPE_COUNT-1)return fail("creative range does not cover every non-air block");
  Inventory creative;
  if(!creative.giveCreative(BlockType::SAND)||creative.cursorStack().count!=Inventory::CREATIVE_STACK_SIZE)return fail("creative give did not create a full cursor stack");
  creative.swapBackpack(0);
  if(!creative.giveCreative(BlockType::SAND)||creative.cursorStack().count!=Inventory::CREATIVE_STACK_SIZE||creative.backpackSlot(0).count!=Inventory::CREATIVE_STACK_SIZE)return fail("creative source was depleted");

  inventory.select(-10); if (inventory.selectedSlot() != 0) return fail("negative selection was not clamped");
  inventory.select(99); if (inventory.selectedSlot() != Inventory::HOTBAR_SLOTS - 1) return fail("high selection was not clamped");
  return 0;
}
