#include "Inventory.h"
#include <algorithm>

template<std::size_t N>
bool addToSlots(std::array<ItemStack,N>& slots,BlockType type){
  for (auto& stack : slots) {
    if (stack.type == type && stack.count > 0 && stack.count < Inventory::MAX_STACK_SIZE) { ++stack.count; return true; }
  }
  for (auto& stack : slots) {
    if (stack.count == 0) { stack = {type, 1}; return true; }
  }
  return false;
}
bool Inventory::addTo(std::array<ItemStack,HOTBAR_SLOTS>& slots,BlockType type){return addToSlots(slots,type);}
bool Inventory::addTo(std::array<ItemStack,BACKPACK_SLOTS>& slots,BlockType type){return addToSlots(slots,type);}

bool Inventory::add(BlockType type){
  if(type==BlockType::AIR||type==BlockType::COUNT)return false;
  return addTo(m_hotbar,type)||addTo(m_backpack,type);
}

bool Inventory::consumeSelected() {
  auto& stack = m_hotbar[m_selected];
  if (stack.count <= 0 || stack.type == BlockType::AIR) return false;
  if (--stack.count == 0) stack.type = BlockType::AIR;
  return true;
}

BlockType Inventory::selectedType() const { return m_hotbar[m_selected].type; }
void Inventory::select(int slot) { m_selected = std::clamp(slot, 0, HOTBAR_SLOTS - 1); }
void Inventory::swapHotbar(int i){if(i>=0&&i<HOTBAR_SLOTS)std::swap(m_hotbar[i],m_cursorStack);}
void Inventory::swapBackpack(int i){if(i>=0&&i<BACKPACK_SLOTS)std::swap(m_backpack[i],m_cursorStack);}
bool Inventory::giveCreative(BlockType type){if(type==BlockType::AIR||type==BlockType::COUNT)return false;m_cursorStack={type,CREATIVE_STACK_SIZE};return true;}
