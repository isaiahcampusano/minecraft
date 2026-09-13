#pragma once
#include <cstdint>
class Chunk;
class WorldGenerator {
public:
  static void generateFlatWorld(Chunk& chunk,std::uint32_t worldSeed=0);
private:
  static void generateTree(Chunk& chunk,int worldX,int worldZ,int surfaceY,std::uint32_t seed);
};

