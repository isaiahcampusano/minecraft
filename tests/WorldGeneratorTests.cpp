#include "world/Chunk.h"
#include "world/WorldGenerator.h"
#include <algorithm>
#include <array>
#include <iostream>

namespace { int fail(const char* message){std::cerr<<message<<'\n';return 1;} }

int main(){
  constexpr std::uint32_t seed=12345u;
  bool foundSand=false,foundGravel=false,foundTree=false,foundFullCanopy=false,foundCrossChunkCanopy=false;
  bool foundCave=false,foundCoal=false,foundIron=false;
  std::array<bool,7> foundHeight{};
  int minSurface=1000,maxSurface=-1000;
  for(int cz=0;cz<8;++cz)for(int cx=0;cx<8;++cx){
    Chunk chunk({cx,cz});WorldGenerator::generateTerrain(chunk,seed);
    for(int z=0;z<Chunk::SIZE_Z;++z)for(int x=0;x<Chunk::SIZE_X;++x){
      const int h=WorldGenerator::surfaceHeightAt(cx*16+x,cz*16+z,seed);
      if(h<28||h>100)return fail("surface height left its documented band");
      minSurface=std::min(minSurface,h);maxSurface=std::max(maxSurface,h);
      if(chunk.getBlock(x,0,z)!=BlockType::BEDROCK)return fail("bedrock was missing at y=0");
      const BlockType surface=chunk.getBlock(x,h,z);
      if(surface!=BlockType::GRASS&&surface!=BlockType::SAND&&surface!=BlockType::GRAVEL)
        return fail("surface block was not grass, sand, or gravel");
      foundSand|=surface==BlockType::SAND;foundGravel|=surface==BlockType::GRAVEL;
      for(int y=1;y<=h-4;++y){
        const BlockType b=chunk.getBlock(x,y,z);
        if(b!=BlockType::STONE&&b!=BlockType::AIR&&b!=BlockType::COAL_ORE&&b!=BlockType::IRON_ORE)
          return fail("subsurface column contained an unexpected block");
        if(b==BlockType::AIR&&y>=3&&y<=h-5)foundCave=true;
        foundCoal|=b==BlockType::COAL_ORE;foundIron|=b==BlockType::IRON_ORE;
        if(b==BlockType::IRON_ORE&&y>52)return fail("iron ore generated above its depth band");
      }
      for(int y=h-3;y<=h-1;++y)if(chunk.getBlock(x,y,z)!=BlockType::DIRT)return fail("dirt layer was malformed");
      // Trees root on grass at the local surface height.
      bool trunk=false;int height=0;
      for(int y=h+1;y<=h+8;++y){if(chunk.getBlock(x,y,z)==BlockType::OAK_LOG){trunk=true;++height;}else if(trunk)break;}
      if(!trunk||surface!=BlockType::GRASS)continue;
      foundTree=true;
      if(height<4||height>6)return fail("tree trunk height was outside the deterministic 4-6 range");
      foundHeight[static_cast<std::size_t>(height)]=true;const int topY=h+height;
      if(x>=2&&x<=13&&z>=2&&z<=13){
        if(chunk.getBlock(x,topY+1,z)!=BlockType::LEAVES)return fail("top canopy leaf was missing");
        for(int dx=-1;dx<=1;++dx)for(int dz=-1;dz<=1;++dz){const BlockType expected=(dx==0&&dz==0)?BlockType::OAK_LOG:BlockType::LEAVES;const BlockType actual=chunk.getBlock(x+dx,topY,z+dz);if(actual!=expected&&!(expected==BlockType::LEAVES&&actual==BlockType::OAK_LOG))return fail("3x3 canopy layer was malformed");}
        for(int dx=-2;dx<=2;++dx)for(int dz=-2;dz<=2;++dz){const BlockType expected=(dx==0&&dz==0)?BlockType::OAK_LOG:BlockType::LEAVES;const BlockType actual=chunk.getBlock(x+dx,topY-1,z+dz);if(actual!=expected&&!(expected==BlockType::LEAVES&&actual==BlockType::OAK_LOG))return fail("5x5 canopy layer was malformed");}
        foundFullCanopy=true;
      }
      if(!foundCrossChunkCanopy&&((x<2&&cx>0)||(x>13&&cx<7)||(z<2&&cz>0)||(z>13&&cz<7))){
        int leafWorldX=cx*16+x,leafWorldZ=cz*16+z;
        if(x<2)leafWorldX-=2;else if(x>13)leafWorldX+=2;else if(z<2)leafWorldZ-=2;else leafWorldZ+=2;
        Chunk neighbor({leafWorldX/16,leafWorldZ/16});WorldGenerator::generateTerrain(neighbor,seed);
        foundCrossChunkCanopy=neighbor.getBlock(leafWorldX%16,topY-1,leafWorldZ%16)==BlockType::LEAVES;
      }
    }
  }
  if(maxSurface-minSurface<10)return fail("terrain had no meaningful hills");
  if(!foundSand||!foundGravel)return fail("deterministic surface patches were not generated");
  if(!foundCave)return fail("cave carver left no air pockets below the surface");
  if(!foundCoal||!foundIron)return fail("ore veins were not generated");
  if(!foundTree||!foundFullCanopy)return fail("full deterministic oak trees were not generated");
  if(!foundHeight[4]||!foundHeight[5]||!foundHeight[6])return fail("tree height variation did not cover 4, 5, and 6 logs");
  if(!foundCrossChunkCanopy)return fail("tree canopy did not continue across a chunk boundary");
  // Determinism: same seed regenerates the identical chunk; a new seed changes the terrain.
  Chunk a({3,3}),b({3,3}),c({3,3});
  WorldGenerator::generateTerrain(a,seed);WorldGenerator::generateTerrain(b,seed);WorldGenerator::generateTerrain(c,seed+1);
  bool same=true,different=false;
  for(int z=0;z<Chunk::SIZE_Z;++z)for(int x=0;x<Chunk::SIZE_X;++x)for(int y=0;y<Chunk::SIZE_Y;++y){
    same&=a.getBlock(x,y,z)==b.getBlock(x,y,z);
    different|=a.getBlock(x,y,z)!=c.getBlock(x,y,z);
  }
  if(!same)return fail("terrain was not deterministic for a fixed seed");
  if(!different)return fail("terrain did not change with the world seed");
  return 0;
}
