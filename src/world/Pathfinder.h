#pragma once
#include <glm/glm.hpp>
#include <vector>

class World;

class Pathfinder{
public:
  static std::vector<glm::ivec3> findPath(const World& world,const glm::ivec3& start,const glm::ivec3& target,int maxRange=16,int maxVisited=256);
  static bool isWalkable(const World& world,const glm::ivec3& feet);
};
