#pragma once
#include "../world/Block.h"

struct ItemStack {
  BlockType type = BlockType::AIR;
  int count = 0;
};
