#pragma once
#include "../player/Item.h"
#include "../world/PassiveMob.h"
#include "GameMode.h"
#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

struct SaveData {
  struct SlotData {
    ItemKind kind=ItemKind::BLOCK;BlockType blockType=BlockType::AIR;MaterialType materialType=MaterialType::STICK;
    ToolKind toolKind=ToolKind::PICKAXE;ToolTier toolTier=ToolTier::WOOD;FoodType foodType=FoodType::APPLE;std::uint8_t count=0;std::uint16_t durability=0;
  };
  struct EditData { std::int32_t x=0,y=0,z=0; BlockType type=BlockType::AIR; };
  std::int32_t selectedSlot=0;
  GameMode mode=GameMode::Survival;
  SlotData cursorStack{};
  std::array<SlotData,9> hotbar{};
  std::array<SlotData,27> backpack{};
  std::int32_t health=20,hunger=20;
  float saturation=5.f,exhaustion=0.f;
  float spawnX=500.f,spawnY=8.f,spawnZ=500.f;
  std::vector<EditData> edits;
  std::vector<MobData> mobs;
};

class SaveLoad {
public:
  static constexpr std::uint32_t VERSION=6;
  static bool save(const SaveData& data);
  static bool save(const SaveData& data,const std::filesystem::path& path);
  static bool load(SaveData& out);
  static bool load(SaveData& out,const std::filesystem::path& path);
  static std::filesystem::path getSavePath();
};
