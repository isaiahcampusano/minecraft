#pragma once
#include "Block.h"

struct BlockProperties {
  float hardness;
  bool diggable;
};

const BlockProperties& getBlockProperties(BlockType type);
