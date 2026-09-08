#pragma once
#include "Chunk.h"
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

class World {
public:
  struct EditEntry{int x,y,z;BlockType type;};
  void update(const glm::vec3& player);
  void updateLighting();
  void render()const;
  BlockType getBlock(int x,int y,int z)const;
  std::uint8_t skyLight(int x,int y,int z)const;
  bool isChunkLoadedAt(int x,int z)const;
  bool setBlock(int x,int y,int z,BlockType type);
  void loadChunk(int cx,int cz);
  bool unloadChunk(int cx,int cz);
  std::vector<EditEntry> getEditEntries()const;
  void applyEditEntries(const std::vector<EditEntry>& entries);
private:
  struct Key{int x,z;bool operator==(const Key&o)const{return x==o.x&&z==o.z;}};
  struct Hash{std::size_t operator()(const Key&k)const{return(static_cast<std::size_t>(static_cast<unsigned>(k.x))<<32)^static_cast<unsigned>(k.z);}};
  struct BlockKey{int x,y,z;bool operator==(const BlockKey&o)const{return x==o.x&&y==o.y&&z==o.z;}};
  struct BlockHash{std::size_t operator()(const BlockKey&k)const;};
  std::unordered_map<Key,std::unique_ptr<Chunk>,Hash> m_chunks; int m_renderDistance=4;
  std::unordered_map<BlockKey,BlockType,BlockHash> m_edits;
  static int floorDiv(int value,int divisor);
  Chunk* find(int cx,int cz); const Chunk* find(int cx,int cz)const;
  void markNeighbors(int cx,int cz);
};

