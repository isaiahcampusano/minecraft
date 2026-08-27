#include "GameState.h"
#include "../player/Inventory.h"
#include "../world/World.h"

namespace {
SaveData::SlotData saveSlot(const ItemStack& stack){return{stack.type,static_cast<std::uint8_t>(stack.count)};}
ItemStack itemStack(const SaveData::SlotData& slot){return{slot.type,slot.count};}
}

SaveData GameState::capture(const Inventory& inventory,const World& world){
  SaveData data;data.selectedSlot=inventory.selectedSlot();data.cursorStack=saveSlot(inventory.cursorStack());
  for(int i=0;i<Inventory::HOTBAR_SLOTS;++i)data.hotbar[static_cast<std::size_t>(i)]=saveSlot(inventory.hotbarSlot(i));
  for(int i=0;i<Inventory::BACKPACK_SLOTS;++i)data.backpack[static_cast<std::size_t>(i)]=saveSlot(inventory.backpackSlot(i));
  for(const auto& edit:world.getEditEntries())data.edits.push_back({edit.x,edit.y,edit.z,edit.type});
  return data;
}

void GameState::apply(const SaveData& data,Inventory& inventory,World& world){
  inventory.select(data.selectedSlot);inventory.setCursorStack(itemStack(data.cursorStack));
  for(int i=0;i<Inventory::HOTBAR_SLOTS;++i)inventory.setHotbarSlot(i,itemStack(data.hotbar[static_cast<std::size_t>(i)]));
  for(int i=0;i<Inventory::BACKPACK_SLOTS;++i)inventory.setBackpackSlot(i,itemStack(data.backpack[static_cast<std::size_t>(i)]));
  std::vector<World::EditEntry> edits;edits.reserve(data.edits.size());for(const auto& edit:data.edits)edits.push_back({edit.x,edit.y,edit.z,edit.type});world.applyEditEntries(edits);
}
