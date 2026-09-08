#pragma once
#include "SaveLoad.h"

class Inventory;
class Player;
class PassiveMobSystem;
class World;

class GameState {
public:
  static SaveData capture(const Inventory& inventory,const World& world,const Player& player,const PassiveMobSystem& mobs);
  static void apply(const SaveData& data,Inventory& inventory,World& world,Player& player,PassiveMobSystem& mobs);
};
