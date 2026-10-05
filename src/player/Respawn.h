#pragma once
#include "../world/World.h"
#include <glm/glm.hpp>
#include <cmath>

inline bool validSpawnCoordinates(const glm::vec3& position){return std::isfinite(position.x)&&std::isfinite(position.y)&&std::isfinite(position.z)&&position.x>=0.f&&position.x<1000.f&&position.y>=0.f&&position.y<256.f&&position.z>=0.f&&position.z<1000.f;}
// Scans a column from the sky down so spawns land on the terrain surface, never inside a hill.
inline glm::vec3 surfaceSpawnInColumn(const World& world,int x,int z){
  for(int y=254;y>=1;--y)
    if(isSolid(world.getBlock(x,y-1,z))&&!isSolid(world.getBlock(x,y,z))&&!isSolid(world.getBlock(x,y+1,z)))
      return{static_cast<float>(x)+.5f,static_cast<float>(y),static_cast<float>(z)+.5f};
  return{static_cast<float>(x)+.5f,60.f,static_cast<float>(z)+.5f};
}
inline glm::vec3 safeRespawnPosition(const World& world,glm::vec3 preferred={500.f,60.f,500.f}){
  if(validSpawnCoordinates(preferred)){
    const int x=static_cast<int>(std::floor(preferred.x)),z=static_cast<int>(std::floor(preferred.z));
    const glm::vec3 surface=surfaceSpawnInColumn(world,x,z);
    if(isSolid(world.getBlock(x,static_cast<int>(surface.y)-1,z)))return surface;
  }
  return surfaceSpawnInColumn(world,500,500);
}
