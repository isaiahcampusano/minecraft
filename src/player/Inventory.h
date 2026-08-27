#pragma once
#include "Item.h"
#include <array>

class Inventory {
public:
  static constexpr int HOTBAR_SLOTS = 9;
  static constexpr int BACKPACK_SLOTS = 27;
  static constexpr int CREATIVE_STACK_SIZE = 64;
  static constexpr int MAX_STACK_SIZE = 64;
  static constexpr int SLOT_COUNT = HOTBAR_SLOTS;
  bool add(BlockType type);
  bool consumeSelected();
  BlockType selectedType() const;
  void select(int slot);
  int selectedSlot() const { return m_selected; }
  const ItemStack& hotbarSlot(int i) const { return m_hotbar[i]; }
  const ItemStack& backpackSlot(int i) const { return m_backpack[i]; }
  const ItemStack& cursorStack() const { return m_cursorStack; }
  const ItemStack& slot(int i) const { return hotbarSlot(i); }
  void swapHotbar(int i);
  void swapBackpack(int i);
  bool giveCreative(BlockType type);
private:
  static bool addTo(std::array<ItemStack,HOTBAR_SLOTS>& slots,BlockType type);
  static bool addTo(std::array<ItemStack,BACKPACK_SLOTS>& slots,BlockType type);
  std::array<ItemStack,HOTBAR_SLOTS> m_hotbar{};
  std::array<ItemStack,BACKPACK_SLOTS> m_backpack{};
  ItemStack m_cursorStack{};
  int m_selected = 0;
};
