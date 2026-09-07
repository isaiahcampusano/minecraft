#pragma once
#include "../world/World.h"
#include <glm/glm.hpp>

inline glm::vec3 safeRespawnPosition(const World& world){for(int y=8;y<=20;++y)if(isSolid(world.getBlock(500,y-1,500))&&!isSolid(world.getBlock(500,y,500))&&!isSolid(world.getBlock(500,y+1,500)))return{500.5f,static_cast<float>(y),500.5f};return{500.f,8.f,500.f};}
