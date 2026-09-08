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
void World::loadChunk(int x,int z){if(x<0||z<0||x>62||z>62||find(x,z))return;auto c=std::make_unique<Chunk>(glm::ivec2{x,z});WorldGenerator::generateFlatWorld(*c);for(const auto& edit:m_edits){if(floorDiv(edit.first.x,16)==x&&floorDiv(edit.first.z,16)==z)c->setBlock(edit.first.x-x*16,edit.first.y,edit.first.z-z*16,edit.second);}m_chunks.emplace(Key{x,z},std::move(c));markNeighbors(x,z);}
bool World::unloadChunk(int x,int z){return m_chunks.erase({x,z})>0;}
void World::markNeighbors(int x,int z){static constexpr int d[4][2]={{1,0},{-1,0},{0,1},{0,-1}};for(auto&a:d)if(auto*c=find(x+a[0],z+a[1]))c->markDirty();}
BlockType World::getBlock(int x,int y,int z)const{if(x<0||x>=1000||z<0||z>=1000||y<0||y>=Chunk::SIZE_Y)return BlockType::AIR;int cx=floorDiv(x,16),cz=floorDiv(z,16);auto*c=find(cx,cz);return c?c->getBlock(x-cx*16,y,z-cz*16):BlockType::AIR;}
std::uint8_t World::skyLight(int x,int y,int z)const{if(x<0||x>=1000||z<0||z>=1000||y<0||y>=Chunk::SIZE_Y)return 0;const int cx=floorDiv(x,16),cz=floorDiv(z,16);const auto* chunk=find(cx,cz);return chunk?chunk->skyLight(x-cx*16,y,z-cz*16):0;}
bool World::isChunkLoadedAt(int x,int z)const{return x>=0&&x<1000&&z>=0&&z<1000&&find(floorDiv(x,16),floorDiv(z,16));}
bool World::setBlock(int x,int y,int z,BlockType t){if(x<0||x>=1000||z<0||z>=1000||y<0||y>=Chunk::SIZE_Y)return false;int cx=floorDiv(x,16),cz=floorDiv(z,16);loadChunk(cx,cz);auto*c=find(cx,cz);if(!c)return false;c->setBlock(x-cx*16,y,z-cz*16,t);m_edits[{x,y,z}]=t;if(x%16==0||x%16==15||z%16==0||z%16==15)markNeighbors(cx,cz);return true;}
std::vector<World::EditEntry> World::getEditEntries()const{std::vector<EditEntry> entries;entries.reserve(m_edits.size());for(const auto& edit:m_edits)entries.push_back({edit.first.x,edit.first.y,edit.first.z,edit.second});std::sort(entries.begin(),entries.end(),[](const EditEntry&a,const EditEntry&b){return std::tie(a.x,a.y,a.z)<std::tie(b.x,b.y,b.z);});return entries;}
void World::applyEditEntries(const std::vector<EditEntry>& entries){std::vector<Key> loaded;loaded.reserve(m_chunks.size());for(const auto& chunk:m_chunks)loaded.push_back(chunk.first);m_chunks.clear();m_edits.clear();for(const auto& edit:entries){const auto type=static_cast<std::size_t>(edit.type);if(edit.x<0||edit.x>=1000||edit.z<0||edit.z>=1000||edit.y<0||edit.y>=Chunk::SIZE_Y||type>=BLOCK_TYPE_COUNT)continue;m_edits[{edit.x,edit.y,edit.z}]=edit.type;}for(const auto& key:loaded)loadChunk(key.x,key.z);}
void World::update(const glm::vec3&p){int pcx=std::clamp(floorDiv(static_cast<int>(std::floor(p.x)),16),0,62),pcz=std::clamp(floorDiv(static_cast<int>(std::floor(p.z)),16),0,62);
  for(int z=pcz-m_renderDistance;z<=pcz+m_renderDistance;++z)for(int x=pcx-m_renderDistance;x<=pcx+m_renderDistance;++x)loadChunk(x,z);
  for(auto it=m_chunks.begin();it!=m_chunks.end();){if(std::abs(it->first.x-pcx)>m_renderDistance+1||std::abs(it->first.z-pcz)>m_renderDistance+1)it=m_chunks.erase(it);else ++it;}
  for(auto&pair:m_chunks)if(pair.second->dirty())pair.second->generateMesh([this](int x,int y,int z){return getBlock(x,y,z);});
}
void World::updateLighting(){for(auto& pair:m_chunks)pair.second->computeSkyLight([this](int x,int y,int z){return getBlock(x,y,z);});}
void World::render()const{for(const auto&pair:m_chunks)pair.second->render();}

