#pragma once
#include "Block.h"
#include "ChunkMeshData.h"
#include <array>
#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <utility>

class Chunk {
public:
  static constexpr int SIZE_X=16,SIZE_Y=256,SIZE_Z=16,COUNT=SIZE_X*SIZE_Y*SIZE_Z;
  static constexpr int LIGHT_RADIUS=15;
  explicit Chunk(glm::ivec2 position);
  void setBlock(int x,int y,int z,BlockType type);
  BlockType getBlock(int x,int y,int z)const;
  void computeSkyLight(const std::function<BlockType(int,int,int)>& worldBlock);
  std::uint8_t skyLight(int x,int y,int z)const;
  // Face order: +X, -X, +Y, -Y, +Z, -Z. Includes directional shade.
  float faceShade(int x,int y,int z,int face)const;
  ChunkMeshData buildMeshData(const std::function<BlockType(int,int,int)>& worldBlock, bool useMaxYBound = true);
  void buildScheduledMeshData(const std::function<BlockType(int,int,int)>& worldBlock, bool useMaxYBound = true);
  bool hasPendingMeshData()const{return m_meshDataPending;}
  ChunkMeshData takePendingMeshData(){m_meshDataPending=false;return std::move(m_meshData);}
  bool ready()const{return m_ready;}
  bool meshDirty()const{return m_meshDirty;}
  bool lightingDirty()const{return m_lightingDirty;}
  bool dirty()const{return m_meshDirty||m_lightingDirty;}
  void markDirty(){m_meshDirty=true;}
  void markLightingDirty(){m_lightingDirty=true;m_meshDirty=true;}
  glm::ivec2 position()const{return m_position;}
  const BlockType* blockData()const{return m_blocks.data();}
  // Highest y holding a non-air block, tracked in setBlock so buildMeshData
  // can skip the empty air above the terrain.
  int maxY()const{return m_maxY;}
private:
  static constexpr int LIGHT_X=SIZE_X+2,LIGHT_Z=SIZE_Z+2;
  glm::ivec2 m_position; std::array<BlockType,COUNT> m_blocks{};
  std::array<std::uint8_t,LIGHT_X*SIZE_Y*LIGHT_Z> m_skyLight{};
  ChunkMeshData m_meshData;
  bool m_ready=false,m_meshDirty=true,m_lightingDirty=true,m_meshDataPending=false;
  int m_maxY=-1;
  static int index(int x,int y,int z){return(y*SIZE_Z+z)*SIZE_X+x;}
  static int lightIndex(int x,int y,int z){return(y*LIGHT_Z+z+1)*LIGHT_X+x+1;}
  bool levelEmpty(int y)const{for(int z=0;z<SIZE_Z;++z)for(int x=0;x<SIZE_X;++x)if(m_blocks[index(x,y,z)]!=BlockType::AIR)return false;return true;}
};
