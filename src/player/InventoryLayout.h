#pragma once
#include "Inventory.h"
#include "CraftingLayout.h"
#include <algorithm>

namespace InventoryLayout {
inline constexpr float SLOT_SIZE=48.f;
inline constexpr float GAP=4.f;
inline constexpr float HOTBAR_Y=16.f;
inline constexpr float BACKPACK_Y=68.f;
inline constexpr float PALETTE_WIDTH=148.f;
inline constexpr float PALETTE_HEIGHT=32.f;
inline constexpr int CREATIVE_PAGE_SIZE=9;
inline constexpr float hotbarWidth(){return Inventory::HOTBAR_SLOTS*SLOT_SIZE+(Inventory::HOTBAR_SLOTS-1)*GAP;}
inline float hotbarX(int width){return(width-hotbarWidth())*.5f;}
inline float paletteX(int width){return std::max(8.f,hotbarX(width)-PALETTE_WIDTH-16.f);}

enum class Area{NONE,HOTBAR,BACKPACK,CREATIVE,PERSONAL_CRAFT,TABLE_CRAFT,CRAFT_OUTPUT};
struct Hit{Area area=Area::NONE;int index=-1;};
inline bool contains(float x,float y,float left,float bottom,float width,float height){return x>=left&&x<left+width&&y>=bottom&&y<bottom+height;}
inline Hit hitTest(float x,float yFromTop,int width,int height,bool tableOpen=false){
  const float y=height-yFromTop,startX=hotbarX(width);
  for(int i=0;i<Inventory::HOTBAR_SLOTS;++i)if(contains(x,y,startX+i*(SLOT_SIZE+GAP),HOTBAR_Y,SLOT_SIZE,SLOT_SIZE))return{Area::HOTBAR,i};
  for(int i=0;i<Inventory::BACKPACK_SLOTS;++i){const int row=i/9,col=i%9;if(contains(x,y,startX+col*(SLOT_SIZE+GAP),BACKPACK_Y+row*(SLOT_SIZE+GAP),SLOT_SIZE,SLOT_SIZE))return{Area::BACKPACK,i};}
  const float creativeX=paletteX(width);
  for(int i=0;i<CREATIVE_PAGE_SIZE;++i)if(contains(x,y,creativeX,BACKPACK_Y+i*(PALETTE_HEIGHT+GAP),PALETTE_WIDTH,PALETTE_HEIGHT))return{Area::CREATIVE,i};
  const int craft=CraftingLayout::hitGrid(x,y,width,tableOpen);if(craft>=0)return{tableOpen?Area::TABLE_CRAFT:Area::PERSONAL_CRAFT,craft};
  if(CraftingLayout::hitOutput(x,y,width,tableOpen))return{Area::CRAFT_OUTPUT,0};
  return{};
}
}
