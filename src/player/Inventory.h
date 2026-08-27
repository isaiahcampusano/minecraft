#pragma once
#include "Item.h"
#include <array>

class Inventory {
public:
  static constexpr int SLOT_COUNT = 9;
  bool add(BlockType type);
  bool consumeSelected();
  BlockType selectedType() const;
  void select(int slot);
  int selectedSlot() const { return m_selected; }
  const ItemStack& slot(int i) const { return m_slots[i]; }
private:
  std::array<ItemStack, SLOT_COUNT> m_slots{};
  int m_selected = 0;
};
