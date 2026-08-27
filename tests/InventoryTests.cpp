#include "player/Inventory.h"
#include <iostream>

namespace { int fail(const char* message) { std::cerr << message << '\n'; return 1; } }

int main() {
  Inventory inventory;
  if (!inventory.add(BlockType::STONE) || inventory.slot(0).type != BlockType::STONE || inventory.slot(0).count != 1) return fail("add did not fill the first empty slot");
  if (!inventory.add(BlockType::STONE) || inventory.slot(0).count != 2 || inventory.slot(1).count != 0) return fail("add did not stack a matching item");
  if (!inventory.consumeSelected() || inventory.slot(0).count != 1) return fail("consume did not decrement the selected stack");
  if (!inventory.consumeSelected() || inventory.slot(0).count != 0 || inventory.slot(0).type != BlockType::AIR) return fail("consume did not clear an exhausted stack");
  if (inventory.consumeSelected()) return fail("consume succeeded on an empty slot");

  for (int value = 1; value <= Inventory::SLOT_COUNT; ++value) if (!inventory.add(static_cast<BlockType>(value))) return fail("could not fill hotbar");
  if (inventory.add(static_cast<BlockType>(Inventory::SLOT_COUNT + 1))) return fail("add succeeded when hotbar was full");
  inventory.select(-10); if (inventory.selectedSlot() != 0) return fail("negative selection was not clamped");
  inventory.select(99); if (inventory.selectedSlot() != Inventory::SLOT_COUNT - 1) return fail("high selection was not clamped");
  return 0;
}
