#include "GameState.h"
#include "../player/Inventory.h"
#include "../player/Player.h"
#include "../world/World.h"

namespace {
SaveData::SlotData saveSlot(const ItemStack& stack){SaveData::SlotData slot;slot.kind=stack.kind;slot.blockType=stack.blockType;slot.materialType=stack.materialType;slot.toolKind=stack.toolKind;slot.toolTier=stack.toolTier;slot.foodType=stack.foodType;slot.count=static_cast<std::uint8_t>(stack.count);slot.durability=static_cast<std::uint16_t>(stack.durability);return slot;}
ItemStack itemStack(const SaveData::SlotData& slot){ItemStack stack;stack.kind=slot.kind;stack.blockType=slot.blockType;stack.materialType=slot.materialType;stack.toolKind=slot.toolKind;stack.toolTier=slot.toolTier;stack.foodType=slot.foodType;stack.count=slot.count;stack.durability=slot.durability;return stack;}
}

SaveData GameState::capture(const Inventory& inventory,const World& world,const Player& player){
  SaveData data;data.selectedSlot=inventory.selectedSlot();data.cursorStack=saveSlot(inventory.cursorStack());
  for(int i=0;i<Inventory::HOTBAR_SLOTS;++i)data.hotbar[static_cast<std::size_t>(i)]=saveSlot(inventory.hotbarSlot(i));
  for(int i=0;i<Inventory::BACKPACK_SLOTS;++i)data.backpack[static_cast<std::size_t>(i)]=saveSlot(inventory.backpackSlot(i));
  data.health=player.survival.health();data.hunger=player.survival.hunger();data.saturation=player.survival.saturation();data.exhaustion=player.survival.exhaustion();
  for(const auto& edit:world.getEditEntries())data.edits.push_back({edit.x,edit.y,edit.z,edit.type});
  return data;
}

void GameState::apply(const SaveData& data,Inventory& inventory,World& world,Player& player){
  inventory.select(data.selectedSlot);inventory.setCursorStack(itemStack(data.cursorStack));
  for(int i=0;i<Inventory::HOTBAR_SLOTS;++i)inventory.setHotbarSlot(i,itemStack(data.hotbar[static_cast<std::size_t>(i)]));
  for(int i=0;i<Inventory::BACKPACK_SLOTS;++i)inventory.setBackpackSlot(i,itemStack(data.backpack[static_cast<std::size_t>(i)]));
  player.survival.restore(data.health,data.hunger,data.saturation,data.exhaustion);
  std::vector<World::EditEntry> edits;edits.reserve(data.edits.size());for(const auto& edit:data.edits)edits.push_back({edit.x,edit.y,edit.z,edit.type});world.applyEditEntries(edits);
}
