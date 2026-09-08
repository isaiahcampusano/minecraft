#pragma once
#include "Chunk.h"
#include <glm/glm.hpp>
#include <memory>
#include <cstddef>
#include <unordered_map>
#include <vector>

class World {
public:
  enum class ChunkTaskType{GENERATE,UPDATE_LIGHTING,REBUILD_MESH};
  struct ChunkTask{ChunkTaskType type;glm::ivec2 position;};
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
  bool isChunkReady(int cx,int cz)const;
  std::size_t loadedChunkCount()const{return m_chunks.size();}
  std::size_t pendingTaskCount()const{return m_pendingTasks.size();}
  void setTaskBudgets(int generation,int lighting,int mesh);
  std::vector<EditEntry> getEditEntries()const;
  void applyEditEntries(const std::vector<EditEntry>& entries);
private:
  struct Key{int x,z;bool operator==(const Key&o)const{return x==o.x&&z==o.z;}};
  struct Hash{std::size_t operator()(const Key&k)const{return(static_cast<std::size_t>(static_cast<unsigned>(k.x))<<32)^static_cast<unsigned>(k.z);}};
  struct BlockKey{int x,y,z;bool operator==(const BlockKey&o)const{return x==o.x&&y==o.y&&z==o.z;}};
  struct BlockHash{std::size_t operator()(const BlockKey&k)const;};
  std::unordered_map<Key,std::unique_ptr<Chunk>,Hash> m_chunks; int m_renderDistance=4;
  std::unordered_map<BlockKey,BlockType,BlockHash> m_edits;
  std::vector<ChunkTask> m_pendingTasks;
  int m_generationBudget=1,m_lightingBudget=1,m_meshBudget=1;
  static int floorDiv(int value,int divisor);
  Chunk* find(int cx,int cz); const Chunk* find(int cx,int cz)const;
  void createChunk(int cx,int cz);
  void queueTask(int cx,int cz,ChunkTaskType type);
  bool processNearestTask(ChunkTaskType type,int pcx,int pcz);
  void processPendingTasks(int pcx,int pcz);
  void discardDistantTasks(int pcx,int pcz);
  void markNeighbors(int cx,int cz,bool lighting=false);
};

