#pragma once
#include "Chunk.h"
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>

class World {
public:
  void update(const glm::vec3& player);
  void render()const;
  BlockType getBlock(int x,int y,int z)const;
  bool setBlock(int x,int y,int z,BlockType type);
private:
  struct Key{int x,z;bool operator==(const Key&o)const{return x==o.x&&z==o.z;}};
  struct Hash{std::size_t operator()(const Key&k)const{return(static_cast<std::size_t>(static_cast<unsigned>(k.x))<<32)^static_cast<unsigned>(k.z);}};
  std::unordered_map<Key,std::unique_ptr<Chunk>,Hash> m_chunks; int m_renderDistance=4;
  static int floorDiv(int value,int divisor);
  Chunk* find(int cx,int cz); const Chunk* find(int cx,int cz)const;
  void load(int cx,int cz); void markNeighbors(int cx,int cz);
};

