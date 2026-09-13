#pragma once
#include "../world/World.h"
#include "../world/PassiveMobSystem.h"
#include "../player/Player.h"
#include "../player/Inventory.h"
#include "../player/PlayerCamera.h"
#include "DayNightCycle.h"
#include "WorldRepository.h"
// Owned gameplay state. Application load/unload boundaries replace every member,
// and reset renderer transients, chat, crafting and contextual input together.
struct WorldSession {
  Player player; PlayerCamera camera; World world; Inventory inventory;
  PassiveMobSystem mobs; DayNightCycle dayNight; WorldInfo info; bool active=false;
};
