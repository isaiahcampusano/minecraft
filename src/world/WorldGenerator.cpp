#include "WorldGenerator.h"
#include "Chunk.h"
void WorldGenerator::generateFlatWorld(Chunk& c){
  for(int x=0;x<Chunk::SIZE_X;++x)for(int z=0;z<Chunk::SIZE_Z;++z){int wx=c.position().x*16+x,wz=c.position().y*16+z;if(wx<0||wx>=1000||wz<0||wz>=1000)continue;
    c.setBlock(x,0,z,BlockType::BEDROCK);for(int y=1;y<=3;++y)c.setBlock(x,y,z,BlockType::DIRT);c.setBlock(x,4,z,BlockType::GRASS);}
}

