#pragma once
#include "../world/Block.h"
#include <cstdint>

enum class ItemKind : std::uint8_t { BLOCK, MATERIAL, TOOL, FOOD };
enum class MaterialType : std::uint8_t { STICK, LEATHER, WHITE_WOOL, COUNT };
enum class ToolKind : std::uint8_t { PICKAXE, AXE, SHOVEL, SWORD, HOE, COUNT };
enum class ToolTier : std::uint8_t { WOOD, STONE, COUNT };
enum class FoodType : std::uint8_t { APPLE, RAW_BEEF, RAW_PORKCHOP, RAW_MUTTON, COUNT };

struct ItemStack {
  ItemKind kind = ItemKind::BLOCK;
  BlockType blockType = BlockType::AIR;
  MaterialType materialType = MaterialType::STICK;
  ToolKind toolKind = ToolKind::PICKAXE;
  ToolTier toolTier = ToolTier::WOOD;
  FoodType foodType = FoodType::APPLE;
  int count = 0;
  int durability = 0;
  bool empty() const { return kind==ItemKind::BLOCK?(blockType==BlockType::AIR||count<=0):count<=0; }
  static ItemStack block(BlockType type,int amount=1){ItemStack result;result.blockType=type;result.count=amount;return result;}
  static ItemStack material(MaterialType type,int amount=1){ItemStack result;result.kind=ItemKind::MATERIAL;result.materialType=type;result.count=amount;return result;}
  static ItemStack tool(ToolKind type,ToolTier tier,int durabilityValue){ItemStack result;result.kind=ItemKind::TOOL;result.toolKind=type;result.toolTier=tier;result.count=1;result.durability=durabilityValue;return result;}
  static ItemStack food(FoodType type,int amount=1){ItemStack result;result.kind=ItemKind::FOOD;result.foodType=type;result.count=amount;return result;}
};
inline bool sameItemType(const ItemStack&a,const ItemStack&b){if(a.kind!=b.kind)return false;if(a.kind==ItemKind::BLOCK)return a.blockType==b.blockType;if(a.kind==ItemKind::MATERIAL)return a.materialType==b.materialType;if(a.kind==ItemKind::FOOD)return a.foodType==b.foodType;return a.toolKind==b.toolKind&&a.toolTier==b.toolTier;}
