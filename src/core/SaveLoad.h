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
  struct FurnaceData {
    std::int32_t x=0,y=0,z=0;
    SlotData input,fuel,output;
    double fuelRemaining=0.,fuelDuration=0.,cookProgress=0.;
  };
  std::vector<FurnaceData> furnaces;
  std::uint32_t seed=0;
  float playerX=500.f,playerY=8.f,playerZ=500.f,yaw=-90.f,pitch=-15.f,timeOfDay=.25f;
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
  static constexpr std::uint32_t VERSION=8;
  static bool save(const SaveData& data);
  static bool save(const SaveData& data,const std::filesystem::path& path);
  static bool load(SaveData& out);
  static bool load(SaveData& out,const std::filesystem::path& path);
  static std::filesystem::path getSavePath();
};
