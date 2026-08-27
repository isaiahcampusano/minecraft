#pragma once
#include "Item.h"
#include <array>
#include <cstdint>

struct ItemColor{std::uint8_t r,g,b;};
inline constexpr std::array<const char*,static_cast<std::size_t>(MaterialType::COUNT)> MATERIAL_NAMES={{"STICK"}};
inline constexpr std::array<ItemColor,static_cast<std::size_t>(MaterialType::COUNT)> MATERIAL_COLORS={{{151,104,61}}};
inline constexpr const char* materialName(MaterialType type){const auto i=static_cast<std::size_t>(type);return i<MATERIAL_NAMES.size()?MATERIAL_NAMES[i]:"UNKNOWN";}
inline constexpr ItemColor materialColor(MaterialType type){const auto i=static_cast<std::size_t>(type);return i<MATERIAL_COLORS.size()?MATERIAL_COLORS[i]:ItemColor{255,0,255};}
