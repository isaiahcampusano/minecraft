#include "SettingsState.h"
#include "SaveLoad.h"
#include <array>
#include <fstream>
#include <set>
#include <string>
#include <GLFW/glfw3.h>
#ifdef _WIN32
#include <windows.h>
#endif

namespace {
constexpr int defaultKeys[] = {87, 83, 65, 68, 32, 341, 340, 69, 70, 294, 49, 50, 51, 52, 53, 54, 55, 56, 57};
constexpr const char* MAGIC = "MCSettings 1";
bool valid(const SettingsState& s) {
  if (s.viewDistance < SettingsState::MIN_VIEW_DISTANCE ||
      s.viewDistance > SettingsState::MAX_VIEW_DISTANCE) return false;
  std::set<int> keys;
  for (int key : s.keybindings) if (key < 0 || key > GLFW_KEY_LAST || key == GLFW_KEY_ESCAPE || !keys.insert(key).second) return false;
  return true;
}
}

SettingsState::SettingsState() { reset(); }
void SettingsState::reset() {
  viewDistance = 4;
  for (std::size_t i = 0; i < keybindings.size(); ++i) keybindings[i] = defaultKeys[i];
}
void SettingsState::setViewDistance(int value) {
  viewDistance = value < MIN_VIEW_DISTANCE ? MIN_VIEW_DISTANCE :
                 value > MAX_VIEW_DISTANCE ? MAX_VIEW_DISTANCE : value;
}
int SettingsState::key(KeyAction action) const { return keybindings[static_cast<std::size_t>(action)]; }
int& SettingsState::key(KeyAction action) { return keybindings[static_cast<std::size_t>(action)]; }
const char* SettingsState::actionName(KeyAction action) {
  static constexpr const char* names[] = {"move_forward","move_backward","move_left","move_right",
    "jump","fly_down","sprint","inventory","toggle_fly","toggle_perspective",
    "hotbar_1","hotbar_2","hotbar_3","hotbar_4","hotbar_5","hotbar_6","hotbar_7","hotbar_8","hotbar_9"};
  const auto index = static_cast<std::size_t>(action);
  return index < static_cast<std::size_t>(KeyAction::Count) ? names[index] : "";
}
std::filesystem::path SettingsState::getSettingsPath() { return SaveLoad::getSavePath().parent_path() / "settings.cfg"; }
bool SettingsState::load(SettingsState& out) { return load(out, getSettingsPath()); }
bool SettingsState::load(SettingsState& out, const std::filesystem::path& path) {
  SettingsState parsed;
  std::ifstream in(path);
  if (!in) { out.reset(); return false; }
  std::string line, name;
  int value = 0;
  if (!std::getline(in, line) || line != MAGIC) { out.reset(); return false; }
  std::array<bool, static_cast<std::size_t>(KeyAction::Count)> seen{};
  if (!std::getline(in, line) || line != "view_distance") { out.reset(); return false; }
  if (!(in >> value) || value < MIN_VIEW_DISTANCE || value > MAX_VIEW_DISTANCE) { out.reset(); return false; }
  parsed.viewDistance = value;
  for (std::size_t i = 0; i < parsed.keybindings.size(); ++i) {
    if (!(in >> name >> value) || name != actionName(static_cast<KeyAction>(i)) ||
        value < 0 || value > GLFW_KEY_LAST || value == GLFW_KEY_ESCAPE || seen[i]) { out.reset(); return false; }
    seen[i] = true; parsed.keybindings[i] = value;
  }
  std::string extra;
  if (in >> extra || !valid(parsed)) { out.reset(); return false; }
  out = parsed; return true;
}
bool SettingsState::save(const SettingsState& settings) { return save(settings, getSettingsPath()); }
bool SettingsState::save(const SettingsState& settings, const std::filesystem::path& path) {
  if (!valid(settings)) return false;
  std::error_code error;
  if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path(), error);
  if (error) return false;
  const auto temp = path.string() + ".tmp";
  { std::ofstream out(temp, std::ios::trunc);
    if (!out) return false;
    out << MAGIC << '\n' << "view_distance\n" << settings.viewDistance << '\n';
    for (std::size_t i = 0; i < settings.keybindings.size(); ++i)
      out << actionName(static_cast<KeyAction>(i)) << ' ' << settings.keybindings[i] << '\n';
    out.flush(); if (!out) { std::filesystem::remove(temp, error); return false; }
  }
#ifdef _WIN32
  if (!MoveFileExW(std::filesystem::path(temp).c_str(), path.c_str(),
                   MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    error = std::make_error_code(std::errc::io_error);
#else
  std::filesystem::rename(temp, path, error);
#endif
  if (error) { std::filesystem::remove(temp, error); return false; }
  return true;
}
