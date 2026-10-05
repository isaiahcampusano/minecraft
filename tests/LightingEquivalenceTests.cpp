// Lighting equivalence: the perf pass must not change computed sky light.
// Builds a fixed world, drains generation+lighting, and hashes every loaded
// chunk's full 18x18x256 light window (including the 1-cell halo border).
// The golden value was recorded on main (pre-perf) and must match exactly.
#include "world/World.h"
#include "world/WorldGenerator.h"
#include <cstdint>
#include <cstdio>
#include <iostream>

namespace {
std::uint64_t fnv1a(std::uint64_t h, std::uint8_t b) {
  h ^= b;
  h *= 1099511628211ULL;
  return h;
}
}  // namespace

int main() {
  World world(7);
  world.setViewDistance(4);
  world.setTaskBudgets(64, 64, 0);  // generate + light only; no GL mesh builds
  const glm::vec3 center{500.5f, 70.f, 500.5f};
  for (int i = 0; i < 200; ++i) world.update(center);
  std::uint64_t h = 1469598103934665603ULL;
  int chunks = 0;
  for (int cz = 27; cz <= 35; ++cz)
    for (int cx = 27; cx <= 35; ++cx) {
      if (!world.isChunkLoadedAt(cx * 16 + 8, cz * 16 + 8)) continue;
      ++chunks;
      for (int y = 0; y < 256; ++y)
        for (int lz = -1; lz <= 16; ++lz)
          for (int lx = -1; lx <= 16; ++lx) h = fnv1a(h, world.skyLight(cx * 16 + lx, y, cz * 16 + lz));
    }
  std::printf("chunks=%d hash=%llu\n", chunks, (unsigned long long)h);
  constexpr std::uint64_t GOLDEN = 4639147457794175297ULL;  // recorded on main, reproduced on the perf branch
  if (GOLDEN != 0 && h != GOLDEN) {
    std::cerr << "sky light hash mismatch: lighting output changed\n";
    return 1;
  }
  return 0;
}
