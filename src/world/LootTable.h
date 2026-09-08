#pragma once
#include "Block.h"
#include "PassiveMob.h"
#include "../player/Item.h"
#include <cstdint>
#include <vector>

class LootTable{
public:
  explicit LootTable(std::uint32_t seed=0x9e3779b9u):m_state(seed?seed:1u){}
  static constexpr std::uint32_t APPLE_DROP_DENOMINATOR=200u;
  static bool dropsAppleForRoll(BlockType type,std::uint32_t roll){return type==BlockType::LEAVES&&roll%APPLE_DROP_DENOMINATOR==0;}
  bool dropsApple(BlockType type){return type==BlockType::LEAVES&&dropsAppleForRoll(type,next());}
  std::vector<ItemStack> mobDrops(MobType type,bool baby){std::vector<ItemStack> drops;if(baby)return drops;if(type==MobType::COW){drops.push_back(ItemStack::food(FoodType::RAW_BEEF,range(1,3)));const int leather=range(0,2);if(leather>0)drops.push_back(ItemStack::material(MaterialType::LEATHER,leather));}else if(type==MobType::PIG)drops.push_back(ItemStack::food(FoodType::RAW_PORKCHOP,range(1,3)));else if(type==MobType::SHEEP){drops.push_back(ItemStack::food(FoodType::RAW_MUTTON,range(1,2)));drops.push_back(ItemStack::material(MaterialType::WHITE_WOOL));}return drops;}
private:
  std::uint32_t m_state;
  std::uint32_t next(){m_state^=m_state<<13;m_state^=m_state>>17;m_state^=m_state<<5;return m_state;}
  int range(int minimum,int maximum){return minimum+static_cast<int>(next()%static_cast<std::uint32_t>(maximum-minimum+1));}
};
