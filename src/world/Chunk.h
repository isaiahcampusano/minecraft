#pragma once
#include "Block.h"
#include "../renderer/Mesh.h"
#include <array>
#include <functional>
#include <glm/glm.hpp>

class Chunk {
public:
  static constexpr int SIZE_X=16,SIZE_Y=256,SIZE_Z=16,COUNT=SIZE_X*SIZE_Y*SIZE_Z;
  explicit Chunk(glm::ivec2 position);
  void setBlock(int x,int y,int z,BlockType type);
  BlockType getBlock(int x,int y,int z)const;
  void generateMesh(const std::function<BlockType(int,int,int)>& worldBlock);
  void render()const{m_mesh.render();}
  bool dirty()const{return m_dirty;} void markDirty(){m_dirty=true;}
  glm::ivec2 position()const{return m_position;}
private:
  glm::ivec2 m_position; std::array<BlockType,COUNT> m_blocks{}; Mesh m_mesh; bool m_dirty=true;
  static int index(int x,int y,int z){return(y*SIZE_Z+z)*SIZE_X+x;}
};

