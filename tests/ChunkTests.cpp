// Tests for Chunk max-Y tracking and the mesh neighbour fast path.
#include "world/Chunk.h"
#include "world/WorldGenerator.h"
#include <iostream>

namespace {
int fail(const char* message) {
  std::cerr << message << '\n';
  return 1;
}
}  // namespace

int main() {
  // max-Y tracking through setBlock.
  {
    Chunk c({0, 0});
    if (c.maxY() != -1) return fail("new chunk maxY != -1");
    c.setBlock(3, 10, 4, BlockType::STONE);
    if (c.maxY() != 10) return fail("maxY not raised by setBlock");
    c.setBlock(5, 20, 6, BlockType::DIRT);
    if (c.maxY() != 20) return fail("maxY not raised to 20");
    c.setBlock(5, 20, 6, BlockType::AIR);
    if (c.maxY() != 10) return fail("maxY did not shrink after clearing top block");
    c.setBlock(3, 10, 4, BlockType::AIR);
    if (c.maxY() != -1) return fail("maxY != -1 after clearing all blocks");
    c.setBlock(1, 5, 1, BlockType::STONE);
    c.setBlock(2, 7, 2, BlockType::STONE);
    if (c.maxY() != 7) return fail("maxY wrong after two sets");
    c.setBlock(1, 5, 1, BlockType::AIR);  // not the top: maxY must not move
    if (c.maxY() != 7) return fail("maxY moved when clearing a non-top block");
    c.setBlock(2, 7, 2, BlockType::GRASS);  // replace top: maxY unchanged
    if (c.maxY() != 7) return fail("maxY moved when replacing the top block");
  }
  // Regression: mining one block of a shared top level must not move maxY.
  {
    Chunk c({0, 0});
    for (int z = 0; z < 16; ++z)
      for (int x = 0; x < 16; ++x) c.setBlock(x, 10, z, BlockType::STONE);  // plateau at y=10
    if (c.maxY() != 10) return fail("plateau maxY != 10");
    c.setBlock(3, 10, 4, BlockType::AIR);  // mine one top block
    if (c.maxY() != 10) return fail("maxY dropped below surviving plateau blocks");
    if (c.getBlock(5, 10, 6) != BlockType::STONE) return fail("plateau block lost");
  }
  // max-Y matches the terrain generator's actual highest block.
  {
    Chunk c({31, 31});
    WorldGenerator::generateTerrain(c, 7);
    int expected = -1;
    for (int y = 255; y >= 0 && expected < 0; --y)
      for (int z = 0; z < 16 && expected < 0; ++z)
        for (int x = 0; x < 16; ++x)
          if (c.getBlock(x, y, z) != BlockType::AIR) {
            expected = y;
            break;
          }
    if (c.maxY() != expected) return fail("maxY does not match generated terrain");
  }
  // In-chunk neighbour fast path resolves exactly like the bounds-checked read.
  {
    Chunk c({31, 31});
    WorldGenerator::generateTerrain(c, 7);
    const BlockType* data = c.blockData();
    static constexpr int dirs[6][3] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0},
                                       {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
    for (int y = 0; y <= c.maxY(); ++y)
      for (int z = 0; z < 16; ++z)
        for (int x = 0; x < 16; ++x) {
          if (c.getBlock(x, y, z) == BlockType::AIR) continue;
          for (int f = 0; f < 6; ++f) {
            const int nx = x + dirs[f][0], ny = y + dirs[f][1], nz = z + dirs[f][2];
            const bool inBounds = nx >= 0 && nx < 16 && ny >= 0 && ny < 256 && nz >= 0 && nz < 16;
            // Fast path as written in buildMeshData.
            const BlockType fast =
                inBounds ? data[(ny * 16 + nz) * 16 + nx] : BlockType::AIR;  // AIR = missing-chunk fallback
            // Oracle: the bounds-checked read it must match.
            const BlockType expected = inBounds ? c.getBlock(nx, ny, nz) : BlockType::AIR;
            if (fast != expected) return fail("fast-path neighbour mismatch");
          }
        }
  }
  std::cout << "chunk tests passed\n";
  return 0;
}
