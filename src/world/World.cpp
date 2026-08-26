#include "World.h"
#include "WorldGenerator.h"
#include <algorithm>
#include <cmath>
#include <vector>

int World::floorDiv(int v,int d){int q=v/d,r=v%d;return r<0?q-1:q;}
Chunk* World::find(int x,int z){auto it=m_chunks.find({x,z});return it==m_chunks.end()?nullptr:it->second.get();}
const Chunk* World::find(int x,int z)const{auto it=m_chunks.find({x,z});return it==m_chunks.end()?nullptr:it->second.get();}
void World::load(int x,int z){if(x<0||z<0||x>62||z>62||find(x,z))return;auto c=std::make_unique<Chunk>(glm::ivec2{x,z});WorldGenerator::generateFlatWorld(*c);m_chunks.emplace(Key{x,z},std::move(c));markNeighbors(x,z);}
void World::markNeighbors(int x,int z){static constexpr int d[4][2]={{1,0},{-1,0},{0,1},{0,-1}};for(auto&a:d)if(auto*c=find(x+a[0],z+a[1]))c->markDirty();}
BlockType World::getBlock(int x,int y,int z)const{if(x<0||x>=1000||z<0||z>=1000||y<0||y>=Chunk::SIZE_Y)return BlockType::AIR;int cx=floorDiv(x,16),cz=floorDiv(z,16);auto*c=find(cx,cz);return c?c->getBlock(x-cx*16,y,z-cz*16):BlockType::AIR;}
bool World::setBlock(int x,int y,int z,BlockType t){if(x<0||x>=1000||z<0||z>=1000||y<0||y>=Chunk::SIZE_Y)return false;int cx=floorDiv(x,16),cz=floorDiv(z,16);load(cx,cz);auto*c=find(cx,cz);if(!c)return false;c->setBlock(x-cx*16,y,z-cz*16,t);if(x%16==0||x%16==15||z%16==0||z%16==15)markNeighbors(cx,cz);return true;}
void World::update(const glm::vec3&p){int pcx=std::clamp(floorDiv(static_cast<int>(std::floor(p.x)),16),0,62),pcz=std::clamp(floorDiv(static_cast<int>(std::floor(p.z)),16),0,62);
  for(int z=pcz-m_renderDistance;z<=pcz+m_renderDistance;++z)for(int x=pcx-m_renderDistance;x<=pcx+m_renderDistance;++x)load(x,z);
  for(auto it=m_chunks.begin();it!=m_chunks.end();){if(std::abs(it->first.x-pcx)>m_renderDistance+1||std::abs(it->first.z-pcz)>m_renderDistance+1)it=m_chunks.erase(it);else ++it;}
  for(auto&pair:m_chunks)if(pair.second->dirty())pair.second->generateMesh([this](int x,int y,int z){return getBlock(x,y,z);});
}
void World::render()const{for(const auto&pair:m_chunks)pair.second->render();}

