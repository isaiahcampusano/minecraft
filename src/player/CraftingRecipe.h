#pragma once
#include "Item.h"
#include <array>
#include <vector>

struct RecipeIngredient {
  ItemKind kind=ItemKind::BLOCK;
  BlockType blockType=BlockType::AIR;
  MaterialType materialType=MaterialType::STICK;
  bool empty()const{return kind==ItemKind::BLOCK&&blockType==BlockType::AIR;}
};

struct CraftingRecipe {
  bool shapeless=false;
  int width=1,height=1;
  std::array<RecipeIngredient,9> grid{};
  ItemStack output{};
  bool requiresTable()const{return width>2||height>2;}
};

class CraftingRegistry {
public:
  static const std::vector<CraftingRecipe>& recipes();
  static const CraftingRecipe* match(const std::array<ItemStack,9>& grid,int width,int height);
  static bool craft(std::array<ItemStack,9>& grid,int width,int height,ItemStack& cursor);
};
