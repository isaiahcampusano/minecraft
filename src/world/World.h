#pragma once
#include "BlockProperties.h"
#include "Chunk.h"
#include "FurnaceState.h"
#include <functional>
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
  struct ChunkMeshUpload{glm::ivec2 position;ChunkMeshData mesh;int maxY;};
  // Direct sampler for the 3x3 chunk neighbourhood, replacing the
  // std::function + unordered_map lookup in the lighting hot path.
  // Fallback semantics match lightingBlock exactly.
  struct LightSampler{
    const BlockType* c[3][3]={};
    int baseX=0,baseZ=0;
    BlockType at(int wx,int y,int wz)const{
      if(y<0)return BlockType::BEDROCK;
      if(y>=Chunk::SIZE_Y||wx<0||wx>=1000||wz<0||wz>=1000)return BlockType::AIR;
      const int lx=(wx-baseX)/Chunk::SIZE_X,lz=(wz-baseZ)/Chunk::SIZE_Z;
      const BlockType* chunk=c[lz][lx];
      if(!chunk)return BlockType::BEDROCK;
      const int bx=wx-(baseX+lx*Chunk::SIZE_X),bz=wz-(baseZ+lz*Chunk::SIZE_Z);
      return chunk[(y*Chunk::SIZE_Z+bz)*Chunk::SIZE_X+bx];
    }
  };
  explicit World(std::uint32_t seed=0):m_seed(seed){}
  std::uint32_t seed()const{return m_seed;}
  void reset(std::uint32_t seed=0){m_chunks.clear();m_edits.clear();m_furnaces.clear();m_pendingTasks.clear();m_seed=seed;}
  void update(const glm::vec3& player);
  void updateLighting();
  std::vector<ChunkMeshUpload> takeChunkMeshUploads();
  std::vector<glm::ivec2> loadedChunkPositions()const;
  BlockType getBlock(int x,int y,int z)const;
  const BlockProperties& getBlockProperties(int x,int y,int z)const;
  std::uint8_t skyLight(int x,int y,int z)const;
  bool isChunkLoadedAt(int x,int z)const;
  using FurnaceDropHandler=std::function<void(const glm::vec3&,const ItemStack&)>;
  struct FurnaceEntry { int x,y,z; FurnaceState state; };
  bool setBlock(int x,int y,int z,BlockType type,const FurnaceDropHandler& dropContents={});
  const FurnaceState* furnaceAt(const glm::ivec3& position)const;
  bool interactFurnace(const glm::ivec3& position,FurnaceSlot slot,ItemStack& cursor);
  void tickFurnaces(double dt);
  std::vector<FurnaceEntry> captureFurnaces()const;
  bool restoreFurnaces(const std::vector<FurnaceEntry>& entries);
  void visitLoadedFurnaces(const std::function<void(const glm::ivec3&,const FurnaceState&)>& visitor)const;
  void loadChunk(int cx,int cz);
  bool unloadChunk(int cx,int cz);
  bool isChunkReady(int cx,int cz)const;
  std::size_t loadedChunkCount()const{return m_chunks.size();}
  std::size_t pendingTaskCount()const{return m_pendingTasks.size();}
  void setTaskBudgets(int generation,int lighting,int mesh);
  int viewDistance() const { return m_renderDistance; }
  void setViewDistance(int distance);
  std::vector<EditEntry> getEditEntries()const;
  void applyEditEntries(const std::vector<EditEntry>& entries);
  // Test hooks (visual-equivalence suite): rebuild all chunk meshes with or
  // without the maxY bound, and inspect per-chunk state.
  void debugRebuildMeshes(bool useMaxYBound);
  struct DebugChunkInfo{int cx,cz;int maxY;bool meshDirty,lightingDirty;};
  std::vector<DebugChunkInfo> debugChunkInfo()const;
private:
  struct Key{int x,z;bool operator==(const Key&o)const{return x==o.x&&z==o.z;}};
  struct Hash{std::size_t operator()(const Key&k)const{return(static_cast<std::size_t>(static_cast<unsigned>(k.x))<<32)^static_cast<unsigned>(k.z);}};
  struct BlockKey{int x,y,z;bool operator==(const BlockKey&o)const{return x==o.x&&y==o.y&&z==o.z;}};
  struct BlockHash{std::size_t operator()(const BlockKey&k)const;};
  std::unordered_map<Key,std::unique_ptr<Chunk>,Hash> m_chunks; int m_renderDistance=4;
  std::unordered_map<BlockKey,BlockType,BlockHash> m_edits;
  std::unordered_map<BlockKey,FurnaceState,BlockHash> m_furnaces;
  std::vector<ChunkTask> m_pendingTasks;
  std::uint32_t m_seed=0;
  int m_generationBudget=1,m_lightingBudget=1,m_meshBudget=1;
  static int floorDiv(int value,int divisor);
  Chunk* find(int cx,int cz); const Chunk* find(int cx,int cz)const;
  void createChunk(int cx,int cz);
  void queueTask(int cx,int cz,ChunkTaskType type);
  bool processNearestTask(ChunkTaskType type,int pcx,int pcz);
  void processPendingTasks(int pcx,int pcz);
  void discardDistantTasks(int pcx,int pcz);
  void markNeighbors(int cx,int cz);
  void invalidateLightingAt(int x,int z);
  BlockType lightingBlock(int x,int y,int z)const;
  LightSampler lightSampler(int cx,int cz)const;
  // Deferred lighting: a chunk's light is only computed once its 3x3
  // neighbourhood exists, killing the relight cascade.
  bool lightingReady(int cx,int cz,int pcx,int pcz)const;
};
