#pragma once
#include "Item.h"
#include <array>

struct SmeltingRecipe {
  FoodType input, output;
  double cookTimeSeconds=10.;
};
class SmeltingRegistry {
public:
  static const std::array<SmeltingRecipe,3>& recipes(){
    static const std::array<SmeltingRecipe,3> value={{{FoodType::RAW_BEEF,FoodType::COOKED_BEEF},
      {FoodType::RAW_PORKCHOP,FoodType::COOKED_PORKCHOP},{FoodType::RAW_MUTTON,FoodType::COOKED_MUTTON}}};
    return value;
  }
  static const SmeltingRecipe* match(FoodType input){for(const auto& recipe:recipes())if(recipe.input==input)return &recipe;return nullptr;}
  static const SmeltingRecipe* match(const ItemStack& input){return !input.empty()&&input.kind==ItemKind::FOOD?match(input.foodType):nullptr;}
};
