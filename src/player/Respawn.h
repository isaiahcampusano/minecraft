#pragma once
#include "../world/World.h"
#include <glm/glm.hpp>
#include <cmath>

inline bool validSpawnCoordinates(const glm::vec3& position){return std::isfinite(position.x)&&std::isfinite(position.y)&&std::isfinite(position.z)&&position.x>=0.f&&position.x<1000.f&&position.y>=0.f&&position.y<256.f&&position.z>=0.f&&position.z<1000.f;}
inline glm::vec3 safeRespawnPosition(const World& world,glm::vec3 preferred={500.f,8.f,500.f}){
  if(validSpawnCoordinates(preferred)){const int x=static_cast<int>(std::floor(preferred.x)),y=static_cast<int>(std::floor(preferred.y)),z=static_cast<int>(std::floor(preferred.z));if(isSolid(world.getBlock(x,y-1,z))&&!isSolid(world.getBlock(x,y,z))&&!isSolid(world.getBlock(x,y+1,z)))return{static_cast<float>(x)+.5f,static_cast<float>(y),static_cast<float>(z)+.5f};}
  for(int y=8;y<=20;++y)if(isSolid(world.getBlock(500,y-1,500))&&!isSolid(world.getBlock(500,y,500))&&!isSolid(world.getBlock(500,y+1,500)))return{500.5f,static_cast<float>(y),500.5f};
  return{500.f,8.f,500.f};
}
