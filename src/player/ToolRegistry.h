#pragma once
#include "MaterialRegistry.h"
#include <array>

inline constexpr std::array<const char*,static_cast<std::size_t>(ToolKind::COUNT)> TOOL_KIND_NAMES={{"PICKAXE","AXE","SHOVEL","SWORD","HOE"}};
inline constexpr std::array<const char*,static_cast<std::size_t>(ToolTier::COUNT)> TOOL_TIER_NAMES={{"WOOD","STONE"}};
inline constexpr std::array<ItemColor,static_cast<std::size_t>(ToolTier::COUNT)> TOOL_TIER_COLORS={{{166,116,67},{135,135,140}}};
inline constexpr std::array<int,static_cast<std::size_t>(ToolTier::COUNT)> TOOL_DURABILITY={{60,130}};
inline constexpr std::array<std::array<int,static_cast<std::size_t>(ToolTier::COUNT)>,static_cast<std::size_t>(ToolKind::COUNT)> TOOL_ATTACK_DAMAGE={{
  {{2,3}},{{2,3}},{{2,3}},{{4,5}},{{2,3}}
}};
inline constexpr const char* toolKindName(ToolKind kind){const auto i=static_cast<std::size_t>(kind);return i<TOOL_KIND_NAMES.size()?TOOL_KIND_NAMES[i]:"UNKNOWN";}
inline constexpr const char* toolTierName(ToolTier tier){const auto i=static_cast<std::size_t>(tier);return i<TOOL_TIER_NAMES.size()?TOOL_TIER_NAMES[i]:"UNKNOWN";}
inline constexpr ItemColor toolColor(ToolKind,ToolTier tier){const auto i=static_cast<std::size_t>(tier);return i<TOOL_TIER_COLORS.size()?TOOL_TIER_COLORS[i]:ItemColor{255,0,255};}
inline constexpr int maxToolDurability(ToolTier tier){const auto i=static_cast<std::size_t>(tier);return i<TOOL_DURABILITY.size()?TOOL_DURABILITY[i]:0;}
inline constexpr int toolAttackDamage(ToolKind kind,ToolTier tier){const auto k=static_cast<std::size_t>(kind),t=static_cast<std::size_t>(tier);return k<TOOL_ATTACK_DAMAGE.size()&&t<TOOL_ATTACK_DAMAGE[k].size()?TOOL_ATTACK_DAMAGE[k][t]:1;}
