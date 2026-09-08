#pragma once
#include "Block.h"
#include <cstdint>

class LootTable{
public:
  explicit LootTable(std::uint32_t seed=0x9e3779b9u):m_state(seed?seed:1u){}
  static constexpr std::uint32_t APPLE_DROP_DENOMINATOR=200u;
  static bool dropsAppleForRoll(BlockType type,std::uint32_t roll){return type==BlockType::LEAVES&&roll%APPLE_DROP_DENOMINATOR==0;}
  bool dropsApple(BlockType type){if(type!=BlockType::LEAVES)return false;m_state^=m_state<<13;m_state^=m_state>>17;m_state^=m_state<<5;return dropsAppleForRoll(type,m_state);}
private:std::uint32_t m_state;
};
