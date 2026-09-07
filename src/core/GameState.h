#pragma once
#include "SaveLoad.h"

class Inventory;
class Player;
class World;

class GameState {
public:
  static SaveData capture(const Inventory& inventory,const World& world,const Player& player);
  static void apply(const SaveData& data,Inventory& inventory,World& world,Player& player);
};
