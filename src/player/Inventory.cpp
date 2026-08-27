#include "Inventory.h"
#include <algorithm>

bool Inventory::add(BlockType type) {
  if (type == BlockType::AIR) return false;
  for (auto& stack : m_slots) {
    if (stack.type == type && stack.count > 0) { ++stack.count; return true; }
  }
  for (auto& stack : m_slots) {
    if (stack.count == 0) { stack = {type, 1}; return true; }
  }
  return false;
}

bool Inventory::consumeSelected() {
  auto& stack = m_slots[m_selected];
  if (stack.count <= 0 || stack.type == BlockType::AIR) return false;
  if (--stack.count == 0) stack.type = BlockType::AIR;
  return true;
}

BlockType Inventory::selectedType() const { return m_slots[m_selected].type; }
void Inventory::select(int slot) { m_selected = std::clamp(slot, 0, SLOT_COUNT - 1); }
