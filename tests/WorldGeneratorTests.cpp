#include "world/Chunk.h"
#include "world/WorldGenerator.h"
#include <array>
#include <iostream>

namespace { int fail(const char* message){std::cerr<<message<<'\n';return 1;} }

int main(){
  bool foundSand=false,foundGravel=false,foundTree=false,foundFullCanopy=false,foundCrossChunkCanopy=false;
  std::array<bool,7> foundHeight{};
  for(int cz=0;cz<8;++cz)for(int cx=0;cx<8;++cx){
    Chunk chunk({cx,cz});WorldGenerator::generateFlatWorld(chunk);
    for(int z=0;z<Chunk::SIZE_Z;++z)for(int x=0;x<Chunk::SIZE_X;++x){
      if(chunk.getBlock(x,0,z)!=BlockType::BEDROCK)return fail("bedrock was missing at y=0");
      if(chunk.getBlock(x,1,z)!=BlockType::STONE||chunk.getBlock(x,2,z)!=BlockType::STONE)return fail("stone layer was missing below dirt");
      if(chunk.getBlock(x,3,z)!=BlockType::DIRT||chunk.getBlock(x,5,z)!=BlockType::DIRT)return fail("dirt layer was malformed");
      const BlockType surface=chunk.getBlock(x,6,z);foundSand|=surface==BlockType::SAND;foundGravel|=surface==BlockType::GRAVEL;
      if(chunk.getBlock(x,7,z)!=BlockType::OAK_LOG)continue;
      foundTree=true;int height=0;while(chunk.getBlock(x,7+height,z)==BlockType::OAK_LOG)++height;
      if(height<4||height>6)return fail("tree trunk height was outside the deterministic 4-6 range");
      foundHeight[static_cast<std::size_t>(height)]=true;const int topY=6+height;
      if(x>=2&&x<=13&&z>=2&&z<=13){
        if(chunk.getBlock(x,topY+1,z)!=BlockType::LEAVES)return fail("top canopy leaf was missing");
        for(int dx=-1;dx<=1;++dx)for(int dz=-1;dz<=1;++dz){const BlockType expected=(dx==0&&dz==0)?BlockType::OAK_LOG:BlockType::LEAVES;const BlockType actual=chunk.getBlock(x+dx,topY,z+dz);if(actual!=expected&&!(expected==BlockType::LEAVES&&actual==BlockType::OAK_LOG))return fail("3x3 canopy layer was malformed");}
        for(int dx=-2;dx<=2;++dx)for(int dz=-2;dz<=2;++dz){const BlockType expected=(dx==0&&dz==0)?BlockType::OAK_LOG:BlockType::LEAVES;const BlockType actual=chunk.getBlock(x+dx,topY-1,z+dz);if(actual!=expected&&!(expected==BlockType::LEAVES&&actual==BlockType::OAK_LOG))return fail("5x5 canopy layer was malformed");}
        foundFullCanopy=true;
      }
      if(!foundCrossChunkCanopy&&((x<2&&cx>0)||(x>13&&cx<62)||(z<2&&cz>0)||(z>13&&cz<62))){
        int leafWorldX=cx*16+x,leafWorldZ=cz*16+z;
        if(x<2)leafWorldX-=2;else if(x>13)leafWorldX+=2;else if(z<2)leafWorldZ-=2;else leafWorldZ+=2;
        Chunk neighbor({leafWorldX/16,leafWorldZ/16});WorldGenerator::generateFlatWorld(neighbor);
        foundCrossChunkCanopy=neighbor.getBlock(leafWorldX%16,topY-1,leafWorldZ%16)==BlockType::LEAVES;
      }
    }
  }
  if(!foundSand||!foundGravel)return fail("deterministic surface patches were not generated");
  if(!foundTree||!foundFullCanopy)return fail("full deterministic oak trees were not generated");
  if(!foundHeight[4]||!foundHeight[5]||!foundHeight[6])return fail("tree height variation did not cover 4, 5, and 6 logs");
  if(!foundCrossChunkCanopy)return fail("tree canopy did not continue across a chunk boundary");
  return 0;
}
