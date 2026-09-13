#pragma once
#include "Item.h"

class FuelRegistry {
public:
  static double burnTime(const ItemStack& item){
    if(item.empty())return 0.;
    if(item.kind==ItemKind::BLOCK&&(item.blockType==BlockType::PLANKS||item.blockType==BlockType::OAK_LOG))return 15.;
    return item.kind==ItemKind::MATERIAL&&item.materialType==MaterialType::STICK?5.:0.;
  }
};
