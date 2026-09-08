#include "world/Chunk.h"
#include <iostream>

namespace { int fail(const char* message){std::cerr<<message<<'\n';return 1;} }

int main(){
  Chunk chunk({0,0});
  if(chunk.ready()||!chunk.lightingDirty()||!chunk.meshDirty())return fail("new chunk readiness flags were incorrect");
  auto worldBlock=[&](int x,int y,int z){return x>=0&&x<Chunk::SIZE_X&&z>=0&&z<Chunk::SIZE_Z?chunk.getBlock(x,y,z):BlockType::AIR;};
  chunk.setBlock(8,10,8,BlockType::GRASS);
  chunk.computeSkyLight(worldBlock);
  if(chunk.lightingDirty()||!chunk.meshDirty())return fail("lighting recomputation did not clear only the lighting dirty flag");
  if(chunk.skyLight(8,200,8)!=15||chunk.skyLight(8,11,8)!=15||chunk.skyLight(8,10,8)!=15)return fail("Open column did not carry full skylight to the grass surface");

  chunk.setBlock(8,11,8,BlockType::STONE);
  if(!chunk.lightingDirty()||!chunk.meshDirty())return fail("block edit did not invalidate lighting and mesh data");
  chunk.computeSkyLight(worldBlock);
  if(chunk.skyLight(8,10,8)>=15)return fail("A block overhead did not darken the block beneath it");

  Chunk enclosed({0,0});
  auto enclosedWorld=[&](int x,int y,int z){return x>=0&&x<Chunk::SIZE_X&&z>=0&&z<Chunk::SIZE_Z?enclosed.getBlock(x,y,z):BlockType::AIR;};
  const int dirs[6][3]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
  for(const auto& d:dirs)enclosed.setBlock(8+d[0],20+d[1],8+d[2],BlockType::STONE);
  enclosed.computeSkyLight(enclosedWorld);
  if(enclosed.skyLight(8,20,8)!=0)return fail("Enclosed air pocket received skylight");
  return 0;
}
