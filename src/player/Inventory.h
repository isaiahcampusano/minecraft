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
  bool add(const ItemStack& stack);
  bool consumeSelected();
  bool consumeSelectedFood(FoodType type);
  BlockType selectedType() const;
  const ItemStack& selectedStack() const { return m_hotbar[m_selected]; }
  void select(int slot);
  int selectedSlot() const { return m_selected; }
  const ItemStack& hotbarSlot(int i) const { return m_hotbar[i]; }
  const ItemStack& backpackSlot(int i) const { return m_backpack[i]; }
  const ItemStack& cursorStack() const { return m_cursorStack; }
  const ItemStack& slot(int i) const { return hotbarSlot(i); }
  void swapHotbar(int i);
  void swapBackpack(int i);
  void swapCraft(bool table,int i);
  bool craftOutput(bool table);
  const ItemStack& craftSlot(bool table,int i)const;
  ItemStack craftingOutput(bool table)const;
  bool damageSelectedTool();
  bool giveCreative(BlockType type);
  bool giveCreative(const ItemStack& stack);
  void clear();
  void setCursorStack(const ItemStack& stack);
  void setHotbarSlot(int i,const ItemStack& stack);
  void setBackpackSlot(int i,const ItemStack& stack);
private:
  static ItemStack normalized(const ItemStack& stack);
  template<std::size_t N> static bool addTo(std::array<ItemStack,N>& slots,const ItemStack& stack);
  std::array<ItemStack,HOTBAR_SLOTS> m_hotbar{};
  std::array<ItemStack,BACKPACK_SLOTS> m_backpack{};
  ItemStack m_cursorStack{};
  std::array<ItemStack,9> m_personalCraft{};
  std::array<ItemStack,9> m_tableCraft{};
  int m_selected = 0;
};
