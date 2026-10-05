#include "WorldGenerator.h"
#include "Chunk.h"
#include <cmath>
#include <cstdint>

namespace {
constexpr int WORLD_SIZE=1000;
constexpr int MIN_SURFACE_Y=28;
constexpr int MAX_SURFACE_Y=100;
constexpr std::uint32_t CONTINENT_SEED=0x1a2b3c4du;
constexpr std::uint32_t HILLS_SEED=0x5e6f7081u;
constexpr std::uint32_t CAVE_SEED_A=0xc0ffee11u;
constexpr std::uint32_t CAVE_SEED_B=0x5eedbeefu;
constexpr std::uint32_t COAL_SEED=0xc0a1u;
constexpr std::uint32_t IRON_SEED=0x120e55u;

std::uint32_t positionHash(int x,int z,std::uint32_t worldSeed){
  std::uint32_t h=static_cast<std::uint32_t>(x)*374761393u+static_cast<std::uint32_t>(z)*668265263u+0x9e3779b9u;
  h^=worldSeed*0x85ebca6bu;
  h=(h^(h>>13))*1274126177u;
  return h^(h>>16);
}
std::uint32_t positionHash3(int x,int y,int z,std::uint32_t worldSeed){
  std::uint32_t h=static_cast<std::uint32_t>(x)*374761393u+static_cast<std::uint32_t>(y)*2246822519u+static_cast<std::uint32_t>(z)*668265263u+0x9e3779b9u;
  h^=worldSeed*0x85ebca6bu;
  h=(h^(h>>13))*1274126177u;
  return h^(h>>16);
}
float lattice2(int x,int z,std::uint32_t seed){
  return static_cast<float>(positionHash(x,z,seed)>>8)/8388607.5f-1.f;
}
float lattice3(int x,int y,int z,std::uint32_t seed){
  return static_cast<float>(positionHash3(x,y,z,seed)>>8)/8388607.5f-1.f;
}
float smootherstep(float t){return t*t*t*(t*(t*6.f-15.f)+10.f);}
float valueNoise2(float x,float z,std::uint32_t seed){
  const int xi=static_cast<int>(std::floor(x)),zi=static_cast<int>(std::floor(z));
  const float xf=x-static_cast<float>(xi),zf=z-static_cast<float>(zi);
  const float u=smootherstep(xf),v=smootherstep(zf);
  const float a=lattice2(xi,zi,seed),b=lattice2(xi+1,zi,seed),c=lattice2(xi,zi+1,seed),d=lattice2(xi+1,zi+1,seed);
  return a+(b-a)*u+(c-a)*v+(a-b-c+d)*u*v;
}
float valueNoise3(float x,float y,float z,std::uint32_t seed){
  const int xi=static_cast<int>(std::floor(x)),yi=static_cast<int>(std::floor(y)),zi=static_cast<int>(std::floor(z));
  const float xf=x-static_cast<float>(xi),yf=y-static_cast<float>(yi),zf=z-static_cast<float>(zi);
  const float u=smootherstep(xf),v=smootherstep(yf),w=smootherstep(zf);
  const float c000=lattice3(xi,yi,zi,seed),c100=lattice3(xi+1,yi,zi,seed);
  const float c010=lattice3(xi,yi+1,zi,seed),c110=lattice3(xi+1,yi+1,zi,seed);
  const float c001=lattice3(xi,yi,zi+1,seed),c101=lattice3(xi+1,yi,zi+1,seed);
  const float c011=lattice3(xi,yi+1,zi+1,seed),c111=lattice3(xi+1,yi+1,zi+1,seed);
  const float x00=c000+(c100-c000)*u,x10=c010+(c110-c010)*u,x01=c001+(c101-c001)*u,x11=c011+(c111-c011)*u;
  const float y0=x00+(x10-x00)*v,y1=x01+(x11-x01)*v;
  return y0+(y1-y0)*w;
}
// Fractal Brownian motion over 2D value noise; result stays roughly in [-1,1].
float fbm2(float x,float z,std::uint32_t seed){
  float total=0.f,amplitude=1.f,frequency=1.f,normalizer=0.f;
  for(int octave=0;octave<4;++octave){
    total+=amplitude*valueNoise2(x*frequency,z*frequency,seed+static_cast<std::uint32_t>(octave)*0x9e3779b9u);
    normalizer+=amplitude;amplitude*=.5f;frequency*=2.f;
  }
  return total/normalizer;
}
BlockType surfaceAt(int worldX,int worldZ,std::uint32_t worldSeed){
  const std::uint32_t patch=positionHash(worldX/8,worldZ/8,worldSeed)%11u;
  return patch==0?BlockType::SAND:(patch==1?BlockType::GRAVEL:BlockType::GRASS);
}
// Spaghetti-cave carver: tunnels form where two independent noise fields are both near zero.
bool carveCaveAt(int x,int y,int z,std::uint32_t worldSeed){
  if(y<3)return false;
  const float a=valueNoise3(static_cast<float>(x)/26.f,static_cast<float>(y)/20.f,static_cast<float>(z)/26.f,worldSeed^CAVE_SEED_A);
  const float b=valueNoise3(static_cast<float>(x)/26.f,static_cast<float>(y)/20.f,static_cast<float>(z)/26.f,worldSeed^CAVE_SEED_B);
  return a*a+b*b<0.022f;
}
bool coalVeinAt(int x,int y,int z,std::uint32_t worldSeed){
  if(y<4)return false;
  return valueNoise3(static_cast<float>(x)/14.f,static_cast<float>(y)/14.f,static_cast<float>(z)/14.f,worldSeed^COAL_SEED)>0.68f;
}
bool ironVeinAt(int x,int y,int z,std::uint32_t worldSeed){
  if(y<4||y>52)return false;
  return valueNoise3(static_cast<float>(x)/12.f,static_cast<float>(y)/12.f,static_cast<float>(z)/12.f,worldSeed^IRON_SEED)>0.74f;
}
void setWorldBlock(Chunk& chunk,int worldX,int y,int worldZ,BlockType type){
  const int originX=chunk.position().x*Chunk::SIZE_X,originZ=chunk.position().y*Chunk::SIZE_Z;
  const int localX=worldX-originX,localZ=worldZ-originZ;
  if(localX<0||localX>=Chunk::SIZE_X||localZ<0||localZ>=Chunk::SIZE_Z)return;
  if(type==BlockType::LEAVES&&chunk.getBlock(localX,y,localZ)==BlockType::OAK_LOG)return;
  chunk.setBlock(localX,y,localZ,type);
}
}

int WorldGenerator::surfaceHeightAt(int worldX,int worldZ,std::uint32_t worldSeed){
  const float continent=fbm2(static_cast<float>(worldX)/160.f,static_cast<float>(worldZ)/160.f,worldSeed^CONTINENT_SEED);
  const float hills=fbm2(static_cast<float>(worldX)/48.f,static_cast<float>(worldZ)/48.f,worldSeed^HILLS_SEED);
  float height=58.f+26.f*continent+9.f*hills;
  if(height<static_cast<float>(MIN_SURFACE_Y))height=static_cast<float>(MIN_SURFACE_Y);
  if(height>static_cast<float>(MAX_SURFACE_Y))height=static_cast<float>(MAX_SURFACE_Y);
  return static_cast<int>(height);
}

void WorldGenerator::generateTree(Chunk& chunk,int worldX,int worldZ,int surfaceY,std::uint32_t seed){
  const int trunkBase=surfaceY+1,trunkHeight=4+static_cast<int>(seed%3u),topY=trunkBase+trunkHeight-1;
  setWorldBlock(chunk,worldX,topY+1,worldZ,BlockType::LEAVES);
  for(int dx=-1;dx<=1;++dx)for(int dz=-1;dz<=1;++dz)if(dx!=0||dz!=0)setWorldBlock(chunk,worldX+dx,topY,worldZ+dz,BlockType::LEAVES);
  for(int dx=-2;dx<=2;++dx)for(int dz=-2;dz<=2;++dz)if(dx!=0||dz!=0)setWorldBlock(chunk,worldX+dx,topY-1,worldZ+dz,BlockType::LEAVES);
  for(int y=trunkBase;y<=topY;++y)setWorldBlock(chunk,worldX,y,worldZ,BlockType::OAK_LOG);
}

void WorldGenerator::generateTerrain(Chunk& chunk,std::uint32_t worldSeed){
  const int originX=chunk.position().x*Chunk::SIZE_X,originZ=chunk.position().y*Chunk::SIZE_Z;
  for(int x=0;x<Chunk::SIZE_X;++x)for(int z=0;z<Chunk::SIZE_Z;++z){
    const int wx=originX+x,wz=originZ+z;
    if(wx<0||wx>=WORLD_SIZE||wz<0||wz>=WORLD_SIZE)continue;
    const int surfaceY=surfaceHeightAt(wx,wz,worldSeed);
    chunk.setBlock(x,0,z,BlockType::BEDROCK);
    for(int y=1;y<=surfaceY-4;++y)chunk.setBlock(x,y,z,BlockType::STONE);
    for(int y=surfaceY-3;y<=surfaceY-1;++y)chunk.setBlock(x,y,z,BlockType::DIRT);
    chunk.setBlock(x,surfaceY,z,surfaceAt(wx,wz,worldSeed));
    // Carve caves through stone only; the dirt cap and bedrock floor stay intact.
    for(int y=3;y<=surfaceY-5;++y){
      if(chunk.getBlock(x,y,z)!=BlockType::STONE)continue;
      if(carveCaveAt(wx,y,wz,worldSeed))chunk.setBlock(x,y,z,BlockType::AIR);
    }
    // Seed ore veins into the remaining stone.
    for(int y=4;y<=surfaceY-4;++y){
      if(chunk.getBlock(x,y,z)!=BlockType::STONE)continue;
      if(ironVeinAt(wx,y,wz,worldSeed))chunk.setBlock(x,y,z,BlockType::IRON_ORE);
      else if(coalVeinAt(wx,y,wz,worldSeed))chunk.setBlock(x,y,z,BlockType::COAL_ORE);
    }
  }
  for(int wz=originZ-2;wz<originZ+Chunk::SIZE_Z+2;++wz)for(int wx=originX-2;wx<originX+Chunk::SIZE_X+2;++wx){
    if(wx<0||wx>=WORLD_SIZE||wz<0||wz>=WORLD_SIZE||surfaceAt(wx,wz,worldSeed)!=BlockType::GRASS)continue;
    const int surfaceY=surfaceHeightAt(wx,wz,worldSeed);
    const std::uint32_t seed=positionHash(wx,wz,worldSeed);if(seed%257u==0u)generateTree(chunk,wx,wz,surfaceY,seed);
  }
}
