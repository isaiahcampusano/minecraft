#pragma once
#include "MaterialRegistry.h"
#include <array>

struct FoodProperties{int nutrition;float saturation;};
inline constexpr std::array<const char*,static_cast<std::size_t>(FoodType::COUNT)> FOOD_NAMES={{"APPLE"}};
inline constexpr std::array<ItemColor,static_cast<std::size_t>(FoodType::COUNT)> FOOD_COLORS={{{205,45,45}}};
inline constexpr std::array<FoodProperties,static_cast<std::size_t>(FoodType::COUNT)> FOOD_PROPERTIES={{{4,2.4f}}};
inline constexpr const char* foodName(FoodType type){const auto i=static_cast<std::size_t>(type);return i<FOOD_NAMES.size()?FOOD_NAMES[i]:"UNKNOWN";}
inline constexpr ItemColor foodColor(FoodType type){const auto i=static_cast<std::size_t>(type);return i<FOOD_COLORS.size()?FOOD_COLORS[i]:ItemColor{255,0,255};}
inline constexpr FoodProperties foodProperties(FoodType type){const auto i=static_cast<std::size_t>(type);return i<FOOD_PROPERTIES.size()?FOOD_PROPERTIES[i]:FoodProperties{0,0};}
