#pragma once
#include <cstdint>
class Chunk;
class WorldGenerator {
public:
  static void generateTerrain(Chunk& chunk,std::uint32_t worldSeed=0);
  // Deterministic surface height for a world column; drives layering, caves, and trees.
  static int surfaceHeightAt(int worldX,int worldZ,std::uint32_t worldSeed);
private:
  static void generateTree(Chunk& chunk,int worldX,int worldZ,int surfaceY,std::uint32_t seed);
};
