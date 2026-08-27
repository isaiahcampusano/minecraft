#pragma once
#include "SaveLoad.h"

class Inventory;
class World;

class GameState {
public:
  static SaveData capture(const Inventory& inventory,const World& world);
  static void apply(const SaveData& data,Inventory& inventory,World& world);
};
