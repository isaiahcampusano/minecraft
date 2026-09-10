#include "player/InventoryLayout.h"
#include <iostream>

namespace {int fail(const char* message){std::cerr<<message<<'\n';return 1;}}

int main(){
  constexpr int width=1280,height=720;
  const float startX=InventoryLayout::hotbarX(width);
  auto hit=InventoryLayout::hitTest(startX+InventoryLayout::SLOT_SIZE*.5f,height-(InventoryLayout::HOTBAR_Y+InventoryLayout::SLOT_SIZE*.5f),width,height,false);
  if(hit.area!=InventoryLayout::Area::HOTBAR||hit.index!=0)return fail("hotbar slot was not detected from framebuffer coordinates");
  hit=InventoryLayout::hitTest(2.f,2.f,width,height,false);
  if(hit.area!=InventoryLayout::Area::NONE)return fail("outside coordinates returned a hit");
  hit=InventoryLayout::hitTest(CraftingLayout::gridX(width,false)+InventoryLayout::SLOT_SIZE*.5f,height-(CraftingLayout::BOTTOM+InventoryLayout::SLOT_SIZE*.5f),width,height,false);
  if(hit.area!=InventoryLayout::Area::PERSONAL_CRAFT||hit.index!=2)return fail("personal crafting grid slot was not detected");
  hit=InventoryLayout::hitTest(CraftingLayout::gridX(width,true)+InventoryLayout::SLOT_SIZE*.5f,height-(CraftingLayout::BOTTOM+InventoryLayout::SLOT_SIZE*.5f),width,height,true);
  if(hit.area!=InventoryLayout::Area::TABLE_CRAFT||hit.index!=6)return fail("table crafting grid slot was not detected");
  hit=InventoryLayout::hitTest(CraftingLayout::outputX(width)+InventoryLayout::SLOT_SIZE*.5f,height-(CraftingLayout::outputY(true)+InventoryLayout::SLOT_SIZE*.5f),width,height,true);
  if(hit.area!=InventoryLayout::Area::CRAFT_OUTPUT)return fail("crafting output slot was not detected");
  hit=InventoryLayout::hitTest(startX+InventoryLayout::hotbarWidth()+16.f+InventoryLayout::SLOT_SIZE*.5f,height-(InventoryLayout::BACKPACK_Y+InventoryLayout::SLOT_SIZE*.5f),width,height,true);
  if(hit.area!=InventoryLayout::Area::HELD)return fail("held item slot was not detected");
  return 0;
}
