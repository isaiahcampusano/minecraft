#pragma once
#include "../world/Block.h"
#include <array>
#include <cstdint>
#include <filesystem>
#include <vector>

struct SaveData {
  struct SlotData { BlockType type=BlockType::AIR; std::uint8_t count=0; };
  struct EditData { std::int32_t x=0,y=0,z=0; BlockType type=BlockType::AIR; };
  std::int32_t selectedSlot=0;
  SlotData cursorStack{};
  std::array<SlotData,9> hotbar{};
  std::array<SlotData,27> backpack{};
  std::vector<EditData> edits;
};

class SaveLoad {
public:
  static constexpr std::uint32_t VERSION=1;
  static bool save(const SaveData& data);
  static bool save(const SaveData& data,const std::filesystem::path& path);
  static bool load(SaveData& out);
  static bool load(SaveData& out,const std::filesystem::path& path);
  static std::filesystem::path getSavePath();
};
