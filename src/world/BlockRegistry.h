#pragma once
#include "Block.h"
#include <array>
#include <cstdint>

struct BlockColor{std::uint8_t r,g,b;};
inline constexpr std::array<BlockColor,BLOCK_TYPE_COUNT> BLOCK_COLORS={{
  {58,58,60},{105,178,62},{126,78,43},{58,58,60},{100,100,100},
  {150,150,150},{150,100,50},{220,200,150},{140,140,140},{177,218,224},
  {51,153,26}
}};
inline constexpr std::array<const char*,BLOCK_TYPE_COUNT> BLOCK_NAMES={{
  "AIR","GRASS","DIRT","BEDROCK","STONE","COBBLESTONE","OAK LOG","SAND","GRAVEL","GLASS","LEAVES"
}};

class PlaceableBlockRange{
public:
  class Iterator{
  public:
    constexpr explicit Iterator(std::size_t value):m_value(value){}
    constexpr BlockType operator*()const{return static_cast<BlockType>(m_value);}
    constexpr Iterator& operator++(){++m_value;return *this;}
    constexpr bool operator!=(const Iterator& other)const{return m_value!=other.m_value;}
  private:std::size_t m_value;
  };
  constexpr Iterator begin()const{return Iterator(1);}
  constexpr Iterator end()const{return Iterator(BLOCK_TYPE_COUNT);}
  constexpr std::size_t size()const{return BLOCK_TYPE_COUNT-1;}
};
inline constexpr PlaceableBlockRange allPlaceableBlocks(){return {};}
inline constexpr const char* blockName(BlockType type){const auto i=static_cast<std::size_t>(type);return i<BLOCK_NAMES.size()?BLOCK_NAMES[i]:"NONE";}
inline constexpr BlockColor blockColor(BlockType type){const auto i=static_cast<std::size_t>(type);return i<BLOCK_COLORS.size()?BLOCK_COLORS[i]:BLOCK_COLORS[static_cast<std::size_t>(BlockType::BEDROCK)];}
