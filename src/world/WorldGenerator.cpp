#include "WorldGenerator.h"
#include "Chunk.h"
#include <cstdint>

namespace {
std::uint32_t positionHash(int x,int z){
  std::uint32_t h=static_cast<std::uint32_t>(x)*374761393u+static_cast<std::uint32_t>(z)*668265263u+0x9e3779b9u;
  h=(h^(h>>13))*1274126177u;
  return h^(h>>16);
}
}

void WorldGenerator::generateFlatWorld(Chunk& c){
  for(int x=0;x<Chunk::SIZE_X;++x)for(int z=0;z<Chunk::SIZE_Z;++z){int wx=c.position().x*16+x,wz=c.position().y*16+z;if(wx<0||wx>=1000||wz<0||wz>=1000)continue;
    c.setBlock(x,0,z,BlockType::BEDROCK);for(int y=1;y<=2;++y)c.setBlock(x,y,z,BlockType::STONE);for(int y=3;y<=5;++y)c.setBlock(x,y,z,BlockType::DIRT);
    const std::uint32_t patch=positionHash(wx/8,wz/8)%11u;const BlockType surface=patch==0?BlockType::SAND:(patch==1?BlockType::GRAVEL:BlockType::GRASS);c.setBlock(x,6,z,surface);
    if(surface==BlockType::GRASS&&positionHash(wx,wz)%257u==0u)for(int y=7;y<=9;++y)c.setBlock(x,y,z,BlockType::OAK_LOG);}
}

