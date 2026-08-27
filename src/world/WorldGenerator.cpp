#include "WorldGenerator.h"
#include "Chunk.h"
#include <cstdint>

namespace {
constexpr int WORLD_SIZE=1000;
constexpr int SURFACE_Y=6;
std::uint32_t positionHash(int x,int z){
  std::uint32_t h=static_cast<std::uint32_t>(x)*374761393u+static_cast<std::uint32_t>(z)*668265263u+0x9e3779b9u;
  h=(h^(h>>13))*1274126177u;
  return h^(h>>16);
}
BlockType surfaceAt(int worldX,int worldZ){
  const std::uint32_t patch=positionHash(worldX/8,worldZ/8)%11u;
  return patch==0?BlockType::SAND:(patch==1?BlockType::GRAVEL:BlockType::GRASS);
}
void setWorldBlock(Chunk& chunk,int worldX,int y,int worldZ,BlockType type){
  const int originX=chunk.position().x*Chunk::SIZE_X,originZ=chunk.position().y*Chunk::SIZE_Z;
  const int localX=worldX-originX,localZ=worldZ-originZ;
  if(localX<0||localX>=Chunk::SIZE_X||localZ<0||localZ>=Chunk::SIZE_Z)return;
  if(type==BlockType::LEAVES&&chunk.getBlock(localX,y,localZ)==BlockType::OAK_LOG)return;
  chunk.setBlock(localX,y,localZ,type);
}
}

void WorldGenerator::generateTree(Chunk& chunk,int worldX,int worldZ,int surfaceY,std::uint32_t seed){
  const int trunkBase=surfaceY+1,trunkHeight=4+static_cast<int>(seed%3u),topY=trunkBase+trunkHeight-1;
  setWorldBlock(chunk,worldX,topY+1,worldZ,BlockType::LEAVES);
  for(int dx=-1;dx<=1;++dx)for(int dz=-1;dz<=1;++dz)if(dx!=0||dz!=0)setWorldBlock(chunk,worldX+dx,topY,worldZ+dz,BlockType::LEAVES);
  for(int dx=-2;dx<=2;++dx)for(int dz=-2;dz<=2;++dz)if(dx!=0||dz!=0)setWorldBlock(chunk,worldX+dx,topY-1,worldZ+dz,BlockType::LEAVES);
  for(int y=trunkBase;y<=topY;++y)setWorldBlock(chunk,worldX,y,worldZ,BlockType::OAK_LOG);
}

void WorldGenerator::generateFlatWorld(Chunk& c){
  const int originX=c.position().x*Chunk::SIZE_X,originZ=c.position().y*Chunk::SIZE_Z;
  for(int x=0;x<Chunk::SIZE_X;++x)for(int z=0;z<Chunk::SIZE_Z;++z){int wx=originX+x,wz=originZ+z;if(wx<0||wx>=WORLD_SIZE||wz<0||wz>=WORLD_SIZE)continue;
    c.setBlock(x,0,z,BlockType::BEDROCK);for(int y=1;y<=2;++y)c.setBlock(x,y,z,BlockType::STONE);for(int y=3;y<=5;++y)c.setBlock(x,y,z,BlockType::DIRT);
    c.setBlock(x,SURFACE_Y,z,surfaceAt(wx,wz));}
  for(int wz=originZ-2;wz<originZ+Chunk::SIZE_Z+2;++wz)for(int wx=originX-2;wx<originX+Chunk::SIZE_X+2;++wx){
    if(wx<0||wx>=WORLD_SIZE||wz<0||wz>=WORLD_SIZE||surfaceAt(wx,wz)!=BlockType::GRASS)continue;
    const std::uint32_t seed=positionHash(wx,wz);if(seed%257u==0u)generateTree(c,wx,wz,SURFACE_Y,seed);
  }
}

