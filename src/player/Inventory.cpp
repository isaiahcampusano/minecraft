#include "Inventory.h"
#include "CraftingRecipe.h"
#include "ToolRegistry.h"
#include <algorithm>

template<std::size_t N>
bool Inventory::addTo(std::array<ItemStack,N>& slots,const ItemStack& item){
  int capacity=0;for(const auto& stack:slots){if(stack.empty())capacity+=item.kind==ItemKind::TOOL?1:MAX_STACK_SIZE;else if(item.kind!=ItemKind::TOOL&&sameItemType(stack,item))capacity+=MAX_STACK_SIZE-stack.count;}if(capacity<item.count)return false;
  int remaining=item.count;if(item.kind!=ItemKind::TOOL)for(auto& stack:slots){const bool same=stack.count>0&&sameItemType(stack,item);if(same&&stack.count<MAX_STACK_SIZE){const int amount=std::min(remaining,MAX_STACK_SIZE-stack.count);stack.count+=amount;remaining-=amount;if(remaining==0)return true;}}
  for(auto& stack:slots)if(stack.empty()){stack=item;stack.count=item.kind==ItemKind::TOOL?1:std::min(remaining,MAX_STACK_SIZE);remaining-=stack.count;if(remaining==0)return true;}
  return false;
}

bool Inventory::add(BlockType type){
  if(type==BlockType::AIR||type==BlockType::COUNT)return false;
  return add(ItemStack::block(type));
}
bool Inventory::add(const ItemStack& source){ItemStack stack=normalized(source);if(stack.empty())return false;return addTo(m_hotbar,stack)||addTo(m_backpack,stack);}

bool Inventory::consumeSelected() {
  auto& stack = m_hotbar[m_selected];
  if(stack.kind!=ItemKind::BLOCK||stack.empty())return false;
  if(--stack.count==0)stack={};
  return true;
}
bool Inventory::consumeSelectedFood(FoodType type){auto& stack=m_hotbar[m_selected];if(stack.kind!=ItemKind::FOOD||stack.foodType!=type||stack.empty())return false;if(--stack.count==0)stack={};return true;}

BlockType Inventory::selectedType() const {const auto& stack=selectedStack();return stack.kind==ItemKind::BLOCK&&!stack.empty()?stack.blockType:BlockType::AIR;}
void Inventory::select(int slot) { m_selected = std::clamp(slot, 0, HOTBAR_SLOTS - 1); }
void Inventory::swapHotbar(int i){if(i>=0&&i<HOTBAR_SLOTS)std::swap(m_hotbar[i],m_cursorStack);}
void Inventory::swapBackpack(int i){if(i>=0&&i<BACKPACK_SLOTS)std::swap(m_backpack[i],m_cursorStack);}
void Inventory::swapCraft(bool table,int i){
  const int limit=table?9:4;if(i<0||i>=limit)return;auto& slot=(table?m_tableCraft:m_personalCraft)[static_cast<std::size_t>(i)];
  if(m_cursorStack.empty()){std::swap(slot,m_cursorStack);return;}
  const bool same=!slot.empty()&&slot.kind!=ItemKind::TOOL&&sameItemType(slot,m_cursorStack);
  if(slot.empty()||same){if(slot.empty()){slot=m_cursorStack;slot.count=0;}if(slot.count<MAX_STACK_SIZE){++slot.count;if(--m_cursorStack.count==0)m_cursorStack={};}return;}
  std::swap(slot,m_cursorStack);
}
bool Inventory::craftOutput(bool table){return CraftingRegistry::craft(table?m_tableCraft:m_personalCraft,table?3:2,table?3:2,m_cursorStack);}
const ItemStack& Inventory::craftSlot(bool table,int i)const{static const ItemStack empty{};const int limit=table?9:4;if(i<0||i>=limit)return empty;return(table?m_tableCraft:m_personalCraft)[static_cast<std::size_t>(i)];}
ItemStack Inventory::craftingOutput(bool table)const{const auto& grid=table?m_tableCraft:m_personalCraft;const auto* recipe=CraftingRegistry::match(grid,table?3:2,table?3:2);return recipe?recipe->output:ItemStack{};}
bool Inventory::damageSelectedTool(){auto& stack=m_hotbar[m_selected];if(stack.kind!=ItemKind::TOOL||stack.empty())return false;if(--stack.durability<=0)stack={};return true;}
bool Inventory::giveCreative(BlockType type){if(type==BlockType::AIR||type==BlockType::COUNT)return false;m_cursorStack=ItemStack::block(type,CREATIVE_STACK_SIZE);return true;}
bool Inventory::giveCreative(const ItemStack& source){auto stack=normalized(source);if(stack.empty())return false;stack.count=stack.kind==ItemKind::TOOL?1:CREATIVE_STACK_SIZE;m_cursorStack=stack;return true;}
ItemStack Inventory::normalized(const ItemStack& stack){
  if(stack.count<=0)return{};
  if(stack.kind==ItemKind::BLOCK){const auto type=static_cast<std::size_t>(stack.blockType);if(type>=BLOCK_TYPE_COUNT||stack.blockType==BlockType::AIR)return{};auto value=stack;value.count=std::min(stack.count,MAX_STACK_SIZE);value.durability=0;return value;}
  if(stack.kind==ItemKind::MATERIAL){if(static_cast<std::size_t>(stack.materialType)>=static_cast<std::size_t>(MaterialType::COUNT))return{};auto value=stack;value.count=std::min(stack.count,MAX_STACK_SIZE);value.durability=0;return value;}
  if(stack.kind==ItemKind::TOOL){if(static_cast<std::size_t>(stack.toolKind)>=static_cast<std::size_t>(ToolKind::COUNT)||static_cast<std::size_t>(stack.toolTier)>=static_cast<std::size_t>(ToolTier::COUNT)||stack.count!=1||stack.durability<=0||stack.durability>maxToolDurability(stack.toolTier))return{};return stack;}
  if(stack.kind==ItemKind::FOOD){if(static_cast<std::size_t>(stack.foodType)>=static_cast<std::size_t>(FoodType::COUNT))return{};auto value=stack;value.count=std::min(stack.count,MAX_STACK_SIZE);value.durability=0;return value;}
  return{};
}
void Inventory::setCursorStack(const ItemStack& stack){m_cursorStack=normalized(stack);}
void Inventory::setHotbarSlot(int i,const ItemStack& stack){if(i>=0&&i<HOTBAR_SLOTS)m_hotbar[i]=normalized(stack);}
void Inventory::setBackpackSlot(int i,const ItemStack& stack){if(i>=0&&i<BACKPACK_SLOTS)m_backpack[i]=normalized(stack);}
void Inventory::clear(){m_hotbar.fill({});m_backpack.fill({});m_personalCraft.fill({});m_tableCraft.fill({});m_cursorStack={};m_selected=0;}
