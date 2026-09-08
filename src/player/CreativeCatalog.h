#pragma once
#include "FoodRegistry.h"
#include "Inventory.h"
#include "ToolRegistry.h"
#include "../world/BlockRegistry.h"
#include <vector>

inline const std::vector<ItemStack>& creativeCatalog(){static const std::vector<ItemStack> items=[](){std::vector<ItemStack> result;for(BlockType type:allPlaceableBlocks())result.push_back(ItemStack::block(type,Inventory::CREATIVE_STACK_SIZE));for(std::size_t i=0;i<static_cast<std::size_t>(MaterialType::COUNT);++i)result.push_back(ItemStack::material(static_cast<MaterialType>(i),Inventory::CREATIVE_STACK_SIZE));for(std::size_t i=0;i<static_cast<std::size_t>(FoodType::COUNT);++i)result.push_back(ItemStack::food(static_cast<FoodType>(i),Inventory::CREATIVE_STACK_SIZE));return result;}();return items;}
inline const char* itemName(const ItemStack& item){if(item.kind==ItemKind::BLOCK)return blockName(item.blockType);if(item.kind==ItemKind::MATERIAL)return materialName(item.materialType);if(item.kind==ItemKind::FOOD)return foodName(item.foodType);return toolKindName(item.toolKind);}
