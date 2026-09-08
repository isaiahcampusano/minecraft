#pragma once
#include "Block.h"
#include "../player/Item.h"
#include <optional>

struct BlockProperties {
  float hardness;
  bool diggable;
  std::optional<ToolKind> effectiveTool;
  std::optional<ToolTier> minTierToDrop;
};

const BlockProperties& getBlockProperties(BlockType type);
BlockType miningDrop(BlockType mined);
bool toolMatches(const ItemStack& held,const BlockProperties& properties);
float miningSpeedMultiplier(const ItemStack& held,const BlockProperties& properties);
bool canDropBlock(const ItemStack& held,const BlockProperties& properties);
bool canDropBlock(BlockType mined,const ItemStack& held);
