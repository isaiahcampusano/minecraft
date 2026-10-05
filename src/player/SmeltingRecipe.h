#pragma once
#include "Item.h"
#include <array>

struct SmeltingRecipe {
  ItemStack input, output;
  double cookTimeSeconds=10.;
};
class SmeltingRegistry {
public:
  static const std::array<SmeltingRecipe,4>& recipes(){
    static const std::array<SmeltingRecipe,4> value={[](){
      std::array<SmeltingRecipe,4> recipes{};
      recipes[0].input=ItemStack::food(FoodType::RAW_BEEF);recipes[0].output=ItemStack::food(FoodType::COOKED_BEEF);
      recipes[1].input=ItemStack::food(FoodType::RAW_PORKCHOP);recipes[1].output=ItemStack::food(FoodType::COOKED_PORKCHOP);
      recipes[2].input=ItemStack::food(FoodType::RAW_MUTTON);recipes[2].output=ItemStack::food(FoodType::COOKED_MUTTON);
      recipes[3].input=ItemStack::block(BlockType::IRON_ORE);recipes[3].output=ItemStack::material(MaterialType::IRON_INGOT);
      return recipes;
    }()};
    return value;
  }
  static const SmeltingRecipe* match(const ItemStack& input){
    if(input.empty())return nullptr;
    for(const auto& recipe:recipes())if(sameItemType(input,recipe.input))return &recipe;
    return nullptr;
  }
  static bool isProduct(const ItemStack& item){
    if(item.empty())return false;
    for(const auto& recipe:recipes())if(sameItemType(item,recipe.output))return true;
    return false;
  }
};
