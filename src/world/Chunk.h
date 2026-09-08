#pragma once
#include "Block.h"
#include "../renderer/Mesh.h"
#include <array>
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>

class Chunk {
public:
  static constexpr int SIZE_X=16,SIZE_Y=256,SIZE_Z=16,COUNT=SIZE_X*SIZE_Y*SIZE_Z;
  explicit Chunk(glm::ivec2 position);
  void setBlock(int x,int y,int z,BlockType type);
  BlockType getBlock(int x,int y,int z)const;
  void computeSkyLight(const std::function<BlockType(int,int,int)>& worldBlock);
  std::uint8_t skyLight(int x,int y,int z)const;
  void generateMesh(const std::function<BlockType(int,int,int)>& worldBlock);
  void render()const{if(m_ready)m_mesh.render();}
  bool ready()const{return m_ready;}
  bool meshDirty()const{return m_meshDirty;}
  bool lightingDirty()const{return m_lightingDirty;}
  bool dirty()const{return m_meshDirty||m_lightingDirty;}
  void markDirty(){m_meshDirty=true;}
  void markLightingDirty(){m_lightingDirty=true;m_meshDirty=true;}
  glm::ivec2 position()const{return m_position;}
private:
  glm::ivec2 m_position; std::array<BlockType,COUNT> m_blocks{}; std::array<std::uint8_t,COUNT> m_skyLight{}; Mesh m_mesh;
  bool m_ready=false,m_meshDirty=true,m_lightingDirty=true;
  static int index(int x,int y,int z){return(y*SIZE_Z+z)*SIZE_X+x;}
};

