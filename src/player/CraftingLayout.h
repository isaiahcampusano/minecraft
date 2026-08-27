#pragma once
#include "Inventory.h"

namespace CraftingLayout {
inline constexpr float SLOT_SIZE=48.f,GAP=4.f;
inline constexpr float BOTTOM=252.f;
inline float gridX(int width,bool table){const float hotbarWidth=Inventory::HOTBAR_SLOTS*SLOT_SIZE+(Inventory::HOTBAR_SLOTS-1)*GAP;return(width-hotbarWidth)*.5f+(table?70.f:96.f);}
inline float outputX(int width){const float hotbarWidth=Inventory::HOTBAR_SLOTS*SLOT_SIZE+(Inventory::HOTBAR_SLOTS-1)*GAP;return(width-hotbarWidth)*.5f+330.f;}
inline float outputY(bool table){return BOTTOM+(table?SLOT_SIZE+GAP:SLOT_SIZE*.5f+GAP*.5f);}
inline int hitGrid(float x,float y,int width,bool table){const int size=table?3:2;const float left=gridX(width,table);for(int i=0;i<size*size;++i){const int row=i/size,col=i%size;const float sx=left+col*(SLOT_SIZE+GAP),sy=BOTTOM+(size-1-row)*(SLOT_SIZE+GAP);if(x>=sx&&x<sx+SLOT_SIZE&&y>=sy&&y<sy+SLOT_SIZE)return i;}return-1;}
inline bool hitOutput(float x,float y,int width,bool table){const float left=outputX(width),bottom=outputY(table);return x>=left&&x<left+SLOT_SIZE&&y>=bottom&&y<bottom+SLOT_SIZE;}
}
