#pragma once
#include "Frustum.h"
#include "GpuMesh.h"
#include <glm/glm.hpp>
#include <map>
#include <utility>

class World;

class ChunkRenderer {
public:
  // Draws cached chunk meshes, skipping chunks fully outside the frustum.
  void render(World& world, const Frustum& frustum);
private:
  using ChunkKey=std::pair<int,int>;
  struct Entry{ GpuMesh mesh; glm::vec3 mn,mx; };
  std::map<ChunkKey,Entry> m_meshes;
};
