#include "world/Chunk.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace {
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
bool close(float a,float b){return std::abs(a-b)<.00001f;}
}

int main(){try{
  auto chunk=std::make_unique<Chunk>(glm::ivec2{0,0});
  auto air=[](int,int,int){return BlockType::AIR;};
  check(!chunk->ready()&&chunk->lightingDirty()&&chunk->meshDirty(),"initial dirty flags");
  chunk->setBlock(8,10,8,BlockType::GRASS);
  chunk->computeSkyLight(air);
  check(!chunk->lightingDirty()&&chunk->meshDirty(),"lighting dirty flags");
  check(chunk->skyLight(8,200,8)==15&&chunk->skyLight(8,11,8)==15,"open sky");
  check(chunk->skyLight(8,10,8)==0&&close(chunk->faceShade(8,10,8,2),1.f),"opaque interior / exposed top");
  chunk->setBlock(8,11,8,BlockType::STONE);
  check(chunk->lightingDirty()&&chunk->meshDirty(),"edit dirty flags");
  chunk->computeSkyLight(air);
  check(chunk->faceShade(8,10,8,2)<1.f,"covered top");
  check(close(chunk->faceShade(8,10,8,0),.82f),"covered block must retain sunlit side");

  check(isSolid(BlockType::LEAVES)&&skyLightAttenuation(BlockType::LEAVES)==1,"leaf collision / attenuation");
  check(skyLightAttenuation(BlockType::AIR)==0&&skyLightAttenuation(BlockType::GLASS)==15,"air / glass attenuation");
  auto canopy=[](int,int y,int){return y>=20&&y<=22?BlockType::LEAVES:BlockType::AIR;};
  auto leaves=std::make_unique<Chunk>(glm::ivec2{0,0});
  for(int y=20;y<=22;++y)for(int z=0;z<16;++z)for(int x=0;x<16;++x)leaves->setBlock(x,y,z,BlockType::LEAVES);
  leaves->computeSkyLight(canopy);
  check(leaves->skyLight(8,22,8)==14&&leaves->skyLight(8,21,8)==13&&leaves->skyLight(8,20,8)==12,"successive leaf attenuation");
  check(leaves->skyLight(8,19,8)==12,"filtered direct light remains constant through air");
  check(close(leaves->faceShade(8,20,8,3),.55f*12.f/15.f),"canopy underside");

  constexpr int dirs[6][3]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
  constexpr float shades[6]={.82f,.72f,1.f,.55f,.9f,.68f};
  // An asymmetric roof makes the six neighboring samples observably different.
  auto faces=std::make_unique<Chunk>(glm::ivec2{0,0});
  for(int y=10;y<14;++y)faces->setBlock(8,y,8,BlockType::OAK_LOG);
  faces->setBlock(9,11,8,BlockType::STONE);
  faces->computeSkyLight(air);
  for(int f=0;f<6;++f){const auto& d=dirs[f];
    const float expected=shades[f]*std::max(.05f,faces->skyLight(8+d[0],10+d[1],8+d[2])/15.f);
    check(close(faces->faceShade(8,10,8,f),expected),"face sampled wrong neighbor");
  }
  check(faces->faceShade(8,10,8,0)<faces->faceShade(8,10,8,1)/.72f*.82f,"asymmetric side light");
  for(const auto& d:dirs)faces->setBlock(4+d[0],20+d[1],4+d[2],BlockType::STONE);
  faces->computeSkyLight(air);
  check(faces->skyLight(4,20,4)==0,"enclosed cavity");
  check(faces->skyLight(8,-1,8)==0&&faces->skyLight(8,256,8)==15,"vertical limits");
  faces->setBlock(8,255,8,BlockType::STONE);faces->computeSkyLight(air);
  check(close(faces->faceShade(8,255,8,2),1.f),"world ceiling top");

  // Translate the same tree and overhang through an interior, an edge and a corner.
  auto scene=[](int x,int y,int z){
    if(y==0)return BlockType::STONE;
    if(x==0&&z==0&&y>=1&&y<=5)return BlockType::OAK_LOG;
    if(std::abs(x)<=2&&std::abs(z)<=2&&(y==5||y==6))return BlockType::LEAVES;
    if(x>=-4&&x<=4&&z>=-4&&z<=4&&y==9)return BlockType::STONE;
    return BlockType::AIR;
  };
  auto reference=std::make_unique<Chunk>(glm::ivec2{0,0});
  for(int z=0;z<16;++z)for(int y=0;y<256;++y)for(int x=0;x<16;++x)reference->setBlock(x,y,z,scene(x-8,y,z-8));
  reference->computeSkyLight([&](int x,int y,int z){return scene(x-8,y,z-8);});
  for(const auto offset:{glm::ivec2{15,8},glm::ivec2{15,15}}){
    for(int cz=0;cz<=1;++cz)for(int cx=0;cx<=1;++cx){
      auto translated=std::make_unique<Chunk>(glm::ivec2{cx,cz});
      auto at=[&](int x,int y,int z){return scene(x-offset.x,y,z-offset.y);};
      for(int z=0;z<16;++z)for(int y=0;y<256;++y)for(int x=0;x<16;++x)translated->setBlock(x,y,z,at(cx*16+x,y,cz*16+z));
      translated->computeSkyLight(at);
      for(int dz=-4;dz<=4;++dz)for(int dx=-4;dx<=4;++dx){
        int x=offset.x+dx-cx*16,z=offset.y+dz-cz*16;
        if(x<0||x>=16||z<0||z>=16)continue;
        for(int y=1;y<=10;++y){
          check(translated->skyLight(x,y,z)==reference->skyLight(8+dx,y,8+dz),"translated skylight seam");
          for(int f=0;f<6;++f)check(close(translated->faceShade(x,y,z,f),reference->faceShade(8+dx,y,8+dz,f)),"translated face seam");
        }
      }
    }
  }
  const auto start=std::chrono::steady_clock::now();
  for(int i=0;i<20;++i)reference->computeSkyLight([&](int x,int y,int z){return scene(x-8,y,z-8);});
  std::cout<<"Lighting recompute mean: "<<std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/20<<" ms\n";
  return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
