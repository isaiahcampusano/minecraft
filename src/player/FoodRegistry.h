#pragma once
#include "MaterialRegistry.h"
#include <array>

struct FoodProperties{int nutrition;float saturation;};
inline constexpr std::array<const char*,static_cast<std::size_t>(FoodType::COUNT)> FOOD_NAMES={{"APPLE","RAW BEEF","RAW PORKCHOP","RAW MUTTON","COOKED BEEF","COOKED PORKCHOP","COOKED MUTTON"}};
inline constexpr std::array<ItemColor,static_cast<std::size_t>(FoodType::COUNT)> FOOD_COLORS={{{205,45,45},{168,58,61},{236,132,142},{178,78,88},{112,58,34},{161,94,55},{133,72,44}}};
inline constexpr std::array<FoodProperties,static_cast<std::size_t>(FoodType::COUNT)> FOOD_PROPERTIES={{{4,2.4f},{3,1.8f},{3,1.8f},{2,1.2f},{8,4.8f},{8,4.8f},{6,3.6f}}};
inline constexpr const char* foodName(FoodType type){const auto i=static_cast<std::size_t>(type);return i<FOOD_NAMES.size()?FOOD_NAMES[i]:"UNKNOWN";}
inline constexpr ItemColor foodColor(FoodType type){const auto i=static_cast<std::size_t>(type);return i<FOOD_COLORS.size()?FOOD_COLORS[i]:ItemColor{255,0,255};}
inline constexpr FoodProperties foodProperties(FoodType type){const auto i=static_cast<std::size_t>(type);return i<FOOD_PROPERTIES.size()?FOOD_PROPERTIES[i]:FoodProperties{0,0};}
