#pragma once
#include "Block.h"
#include <cstdint>

class LootTable{
public:
  explicit LootTable(std::uint32_t seed=0x9e3779b9u):m_state(seed?seed:1u){}
  bool dropsApple(BlockType type){if(type!=BlockType::LEAVES)return false;m_state^=m_state<<13;m_state^=m_state>>17;m_state^=m_state<<5;return m_state%20u==0;}
private:std::uint32_t m_state;
};
