#pragma once
#include <array>
#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>
#include "GameMode.h"

enum class KeyAction : std::size_t {
  MoveForward, MoveBackward, MoveLeft, MoveRight, Jump, FlyDown, Sprint,
  Inventory, ToggleFly, TogglePerspective, Hotbar1, Hotbar2, Hotbar3,
  Hotbar4, Hotbar5, Hotbar6, Hotbar7, Hotbar8, Hotbar9, Pause, Attack, Use, PickBlock, DropItem, Sneak, OpenChat, ToggleHUD, Zoom, Count,
  Forward = MoveForward, Backward = MoveBackward, Left = MoveLeft, Right = MoveRight
};

class SettingsState {
public:
  using Action = KeyAction;
  static constexpr std::size_t ACTION_COUNT = static_cast<std::size_t>(KeyAction::Count);
  static constexpr int MIN_VIEW_DISTANCE = 2;
  static constexpr int MAX_VIEW_DISTANCE = 8;
  int viewDistance = 4;
  float baseFov=70.f,mouseSensitivity=.15f;
  bool vsync=true,invertY=false,rawMouseInput=true;
  GameMode defaultMode=GameMode::Survival;
  std::string notice;
  static constexpr int MOUSE_OFFSET=1000;
  static bool validCode(int code);
  static std::string bindingName(int code);
  static bool contextsOverlap(KeyAction a,KeyAction b);
  static int defaultBinding(KeyAction action);
  std::vector<KeyAction> conflicts(KeyAction action,int code)const;
  bool assign(KeyAction action,int code,bool swap=false);
  bool valid()const;
  void resetBindings();
  std::array<int, static_cast<std::size_t>(KeyAction::Count)> keybindings{};

  SettingsState();
  void reset();
  void setViewDistance(int value);
  int key(KeyAction action) const;
  int& key(KeyAction action);
  int keyBinding(KeyAction action) const { return key(action); }
  void setKeyBinding(KeyAction action, int value) { key(action) = value; }
  static const char* actionLabel(KeyAction action);
  static const char* actionName(KeyAction action);
  static std::filesystem::path getSettingsPath();
  static bool load(SettingsState& out);
  static bool load(SettingsState& out, const std::filesystem::path& path);
  static bool save(const SettingsState& settings);
  static bool save(const SettingsState& settings, const std::filesystem::path& path);
};
