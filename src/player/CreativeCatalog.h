#pragma once
#include "FoodRegistry.h"
#include "Inventory.h"
#include "ToolRegistry.h"
#include "../world/BlockRegistry.h"
#include <array>

inline const std::array<ItemStack,BLOCK_TYPE_COUNT>& creativeCatalog(){static const std::array<ItemStack,BLOCK_TYPE_COUNT> items=[](){std::array<ItemStack,BLOCK_TYPE_COUNT> result{};std::size_t i=0;for(BlockType type:allPlaceableBlocks())result[i++]=ItemStack::block(type,Inventory::CREATIVE_STACK_SIZE);result[i]=ItemStack::food(FoodType::APPLE,Inventory::CREATIVE_STACK_SIZE);return result;}();return items;}
inline const char* itemName(const ItemStack& item){if(item.kind==ItemKind::BLOCK)return blockName(item.blockType);if(item.kind==ItemKind::MATERIAL)return materialName(item.materialType);if(item.kind==ItemKind::FOOD)return foodName(item.foodType);return toolKindName(item.toolKind);}
