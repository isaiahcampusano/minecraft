#pragma once
#include "Block.h"
#include "../player/Item.h"
#include <optional>

struct BlockProperties {
  float hardness=0.f;
  bool diggable=false;
  std::optional<ToolKind> effectiveTool;
  std::optional<ToolTier> minTierToDrop;
  bool isWalkable=false;
  float movementCost=1.f;
  bool isHazard=false;
};

const BlockProperties& getBlockProperties(BlockType type);
ItemStack miningDrop(BlockType mined);
bool toolMatches(const ItemStack& held,const BlockProperties& properties);
float miningSpeedMultiplier(const ItemStack& held,const BlockProperties& properties);
bool canDropBlock(const ItemStack& held,const BlockProperties& properties);
bool canDropBlock(BlockType mined,const ItemStack& held);
