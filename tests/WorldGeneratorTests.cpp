#include "world/Chunk.h"
#include "world/WorldGenerator.h"
#include <iostream>

namespace { int fail(const char* message){std::cerr<<message<<'\n';return 1;} }

int main(){
  bool foundSand=false,foundGravel=false,foundTree=false;
  for(int cz=0;cz<8;++cz)for(int cx=0;cx<8;++cx){
    Chunk chunk({cx,cz});WorldGenerator::generateFlatWorld(chunk);
    for(int z=0;z<Chunk::SIZE_Z;++z)for(int x=0;x<Chunk::SIZE_X;++x){
      if(chunk.getBlock(x,0,z)!=BlockType::BEDROCK)return fail("bedrock was missing at y=0");
      if(chunk.getBlock(x,1,z)!=BlockType::STONE||chunk.getBlock(x,2,z)!=BlockType::STONE)return fail("stone layer was missing below dirt");
      if(chunk.getBlock(x,3,z)!=BlockType::DIRT||chunk.getBlock(x,5,z)!=BlockType::DIRT)return fail("dirt layer was malformed");
      const BlockType surface=chunk.getBlock(x,6,z);foundSand|=surface==BlockType::SAND;foundGravel|=surface==BlockType::GRAVEL;foundTree|=chunk.getBlock(x,7,z)==BlockType::OAK_LOG;
    }
  }
  if(!foundSand||!foundGravel)return fail("deterministic surface patches were not generated");
  if(!foundTree)return fail("deterministic oak trunks were not generated");
  return 0;
}
