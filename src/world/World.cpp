#include "World.h"
#include "WorldGenerator.h"
#include <algorithm>
#include <cmath>
#include <tuple>
#include <vector>

int World::floorDiv(int v,int d){int q=v/d,r=v%d;return r<0?q-1:q;}
std::size_t World::BlockHash::operator()(const BlockKey& k)const{
  std::size_t h=static_cast<unsigned>(k.x);
  h=(h*16777619u)^static_cast<unsigned>(k.y);
  return(h*16777619u)^static_cast<unsigned>(k.z);
}
Chunk* World::find(int x,int z){auto it=m_chunks.find({x,z});return it==m_chunks.end()?nullptr:it->second.get();}
const Chunk* World::find(int x,int z)const{auto it=m_chunks.find({x,z});return it==m_chunks.end()?nullptr:it->second.get();}
void World::createChunk(int x,int z){if(x<0||z<0||x>62||z>62||find(x,z))return;auto c=std::make_unique<Chunk>(glm::ivec2{x,z});WorldGenerator::generateFlatWorld(*c);for(const auto& edit:m_edits)if(floorDiv(edit.first.x,16)==x&&floorDiv(edit.first.z,16)==z)c->setBlock(edit.first.x-x*16,edit.first.y,edit.first.z-z*16,edit.second);m_chunks.emplace(Key{x,z},std::move(c));markNeighbors(x,z);}
void World::loadChunk(int x,int z){createChunk(x,z);}
bool World::unloadChunk(int x,int z){if(!m_chunks.erase({x,z}))return false;m_pendingTasks.erase(std::remove_if(m_pendingTasks.begin(),m_pendingTasks.end(),[&](const ChunkTask& task){return task.position==glm::ivec2{x,z};}),m_pendingTasks.end());markNeighbors(x,z);return true;}
bool World::isChunkReady(int x,int z)const{const auto* chunk=find(x,z);return chunk&&chunk->ready();}
void World::setTaskBudgets(int generation,int lighting,int mesh){m_generationBudget=std::max(0,generation);m_lightingBudget=std::max(0,lighting);m_meshBudget=std::max(0,mesh);}
void World::setViewDistance(int distance){m_renderDistance=std::clamp(distance,2,8);m_pendingTasks.erase(std::remove_if(m_pendingTasks.begin(),m_pendingTasks.end(),[this](const ChunkTask& task){return std::abs(task.position.x)>63||std::abs(task.position.y)>63;}),m_pendingTasks.end());}
void World::queueTask(int x,int z,ChunkTaskType type){if(x<0||z<0||x>62||z>62)return;const glm::ivec2 position{x,z};if(std::none_of(m_pendingTasks.begin(),m_pendingTasks.end(),[&](const ChunkTask& task){return task.type==type&&task.position==position;}))m_pendingTasks.push_back({type,position});}
void World::markNeighbors(int x,int z){
  for(int dz=-1;dz<=1;++dz)for(int dx=-1;dx<=1;++dx){
    if(dx==0&&dz==0)continue;
    if(auto* chunk=find(x+dx,z+dz)){
      chunk->markLightingDirty();
      queueTask(x+dx,z+dz,ChunkTaskType::UPDATE_LIGHTING);
    }
  }
}
void World::invalidateLightingAt(int x,int z){
  for(auto& entry:m_chunks){
    const int ox=entry.first.x*Chunk::SIZE_X,oz=entry.first.z*Chunk::SIZE_Z;
    if(x<ox-Chunk::LIGHT_RADIUS||x>=ox+Chunk::SIZE_X+Chunk::LIGHT_RADIUS||
       z<oz-Chunk::LIGHT_RADIUS||z>=oz+Chunk::SIZE_Z+Chunk::LIGHT_RADIUS)continue;
    entry.second->markLightingDirty();
    queueTask(entry.first.x,entry.first.z,ChunkTaskType::UPDATE_LIGHTING);
  }
}
BlockType World::lightingBlock(int x,int y,int z)const{
  if(y<0)return BlockType::BEDROCK;
  if(y>=Chunk::SIZE_Y||x<0||x>=1000||z<0||z>=1000)return BlockType::AIR;
  const int cx=floorDiv(x,Chunk::SIZE_X),cz=floorDiv(z,Chunk::SIZE_Z);
  const auto* chunk=find(cx,cz);
  return chunk?chunk->getBlock(x-cx*Chunk::SIZE_X,y,z-cz*Chunk::SIZE_Z):BlockType::BEDROCK;
}
BlockType World::getBlock(int x,int y,int z)const{if(x<0||x>=1000||z<0||z>=1000||y<0||y>=Chunk::SIZE_Y)return BlockType::AIR;int cx=floorDiv(x,16),cz=floorDiv(z,16);auto*c=find(cx,cz);return c?c->getBlock(x-cx*16,y,z-cz*16):BlockType::AIR;}
std::uint8_t World::skyLight(int x,int y,int z)const{if(x<0||x>=1000||z<0||z>=1000||y<0||y>=Chunk::SIZE_Y)return 0;const int cx=floorDiv(x,16),cz=floorDiv(z,16);const auto* chunk=find(cx,cz);return chunk?chunk->skyLight(x-cx*16,y,z-cz*16):0;}
bool World::isChunkLoadedAt(int x,int z)const{return x>=0&&x<1000&&z>=0&&z<1000&&find(floorDiv(x,16),floorDiv(z,16));}
bool World::setBlock(int x,int y,int z,BlockType t){
  if(x<0||x>=1000||z<0||z>=1000||y<0||y>=Chunk::SIZE_Y)return false;
  const int cx=floorDiv(x,16),cz=floorDiv(z,16);
  loadChunk(cx,cz);
  auto* chunk=find(cx,cz);if(!chunk)return false;
  m_edits[{x,y,z}]=t;
  if(chunk->getBlock(x-cx*16,y,z-cz*16)==t)return true;
  chunk->setBlock(x-cx*16,y,z-cz*16,t);
  invalidateLightingAt(x,z);
  return true;
}
std::vector<World::EditEntry> World::getEditEntries()const{std::vector<EditEntry> entries;entries.reserve(m_edits.size());for(const auto& edit:m_edits)entries.push_back({edit.first.x,edit.first.y,edit.first.z,edit.second});std::sort(entries.begin(),entries.end(),[](const EditEntry&a,const EditEntry&b){return std::tie(a.x,a.y,a.z)<std::tie(b.x,b.y,b.z);});return entries;}
void World::applyEditEntries(const std::vector<EditEntry>& entries){std::vector<Key> loaded;loaded.reserve(m_chunks.size());for(const auto& chunk:m_chunks)loaded.push_back(chunk.first);m_chunks.clear();m_pendingTasks.clear();m_edits.clear();for(const auto& edit:entries){const auto type=static_cast<std::size_t>(edit.type);if(edit.x<0||edit.x>=1000||edit.z<0||edit.z>=1000||edit.y<0||edit.y>=Chunk::SIZE_Y||type>=BLOCK_TYPE_COUNT)continue;m_edits[{edit.x,edit.y,edit.z}]=edit.type;}for(const auto& key:loaded)loadChunk(key.x,key.z);}
bool World::processNearestTask(ChunkTaskType type,int pcx,int pcz){auto nearest=m_pendingTasks.end();int best=0;for(auto it=m_pendingTasks.begin();it!=m_pendingTasks.end();++it){if(it->type!=type)continue;const int dx=it->position.x-pcx,dz=it->position.y-pcz,distance=dx*dx+dz*dz;if(nearest==m_pendingTasks.end()||distance<best||(distance==best&&std::tie(it->position.y,it->position.x)<std::tie(nearest->position.y,nearest->position.x))){nearest=it;best=distance;}}if(nearest==m_pendingTasks.end())return false;const ChunkTask task=*nearest;m_pendingTasks.erase(nearest);const int x=task.position.x,z=task.position.y;if(type==ChunkTaskType::GENERATE){if(!find(x,z))createChunk(x,z);if(find(x,z))queueTask(x,z,ChunkTaskType::UPDATE_LIGHTING);}else if(auto* chunk=find(x,z)){if(type==ChunkTaskType::UPDATE_LIGHTING){if(chunk->lightingDirty())chunk->computeSkyLight([this](int wx,int y,int wz){return lightingBlock(wx,y,wz);});if(chunk->meshDirty())queueTask(x,z,ChunkTaskType::REBUILD_MESH);}else if(chunk->lightingDirty())queueTask(x,z,ChunkTaskType::UPDATE_LIGHTING);else if(chunk->meshDirty())chunk->generateMesh([this](int wx,int y,int wz){return getBlock(wx,y,wz);});}return true;}
void World::processPendingTasks(int pcx,int pcz){for(int i=0;i<m_generationBudget&&processNearestTask(ChunkTaskType::GENERATE,pcx,pcz);++i){}for(int i=0;i<m_lightingBudget&&processNearestTask(ChunkTaskType::UPDATE_LIGHTING,pcx,pcz);++i){}for(int i=0;i<m_meshBudget&&processNearestTask(ChunkTaskType::REBUILD_MESH,pcx,pcz);++i){}}
void World::discardDistantTasks(int pcx,int pcz){m_pendingTasks.erase(std::remove_if(m_pendingTasks.begin(),m_pendingTasks.end(),[&](const ChunkTask& task){return std::abs(task.position.x-pcx)>m_renderDistance+1||std::abs(task.position.y-pcz)>m_renderDistance+1;}),m_pendingTasks.end());}
void World::update(const glm::vec3&p){int pcx=std::clamp(floorDiv(static_cast<int>(std::floor(p.x)),16),0,62),pcz=std::clamp(floorDiv(static_cast<int>(std::floor(p.z)),16),0,62);
  for(int z=pcz-m_renderDistance;z<=pcz+m_renderDistance;++z)for(int x=pcx-m_renderDistance;x<=pcx+m_renderDistance;++x)if(!find(x,z))queueTask(x,z,ChunkTaskType::GENERATE);
  std::vector<Key> unload;for(const auto& pair:m_chunks)if(std::abs(pair.first.x-pcx)>m_renderDistance+1||std::abs(pair.first.z-pcz)>m_renderDistance+1)unload.push_back(pair.first);for(const auto& key:unload)unloadChunk(key.x,key.z);
  for(const auto& pair:m_chunks)if(pair.second->lightingDirty())queueTask(pair.first.x,pair.first.z,ChunkTaskType::UPDATE_LIGHTING);else if(pair.second->meshDirty())queueTask(pair.first.x,pair.first.z,ChunkTaskType::REBUILD_MESH);
  discardDistantTasks(pcx,pcz);processPendingTasks(pcx,pcz);
}
void World::updateLighting(){for(auto& pair:m_chunks)if(pair.second->lightingDirty())pair.second->computeSkyLight([this](int x,int y,int z){return lightingBlock(x,y,z);});}
void World::render()const{for(const auto&pair:m_chunks)pair.second->render();}
