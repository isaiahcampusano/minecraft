#include "Chunk.h"
#include <algorithm>
#include <queue>

Chunk::Chunk(glm::ivec2 p):m_position(p){m_blocks.fill(BlockType::AIR);}
void Chunk::setBlock(int x,int y,int z,BlockType t){if(x>=0&&x<SIZE_X&&y>=0&&y<SIZE_Y&&z>=0&&z<SIZE_Z&&m_blocks[index(x,y,z)]!=t){m_blocks[index(x,y,z)]=t;markLightingDirty();}}
BlockType Chunk::getBlock(int x,int y,int z)const{return x>=0&&x<SIZE_X&&y>=0&&y<SIZE_Y&&z>=0&&z<SIZE_Z?m_blocks[index(x,y,z)]:BlockType::AIR;}
namespace {
constexpr int DIRECTIONS[6][3]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
constexpr float FACE_SHADE[6]={.82f,.72f,1.f,.55f,.9f,.68f};
}

std::uint8_t Chunk::skyLight(int x,int y,int z)const{
  if(y<0)return 0;
  if(y>=SIZE_Y)return 15;
  return x>=-1&&x<=SIZE_X&&z>=-1&&z<=SIZE_Z?m_skyLight[lightIndex(x,y,z)]:0;
}

float Chunk::faceShade(int x,int y,int z,int face)const{
  if(face<0||face>=6)return 0.f;
  const auto& d=DIRECTIONS[face];
  return FACE_SHADE[face]*std::max(.05f,skyLight(x+d[0],y+d[1],z+d[2])/15.f);
}

void Chunk::computeSkyLight(const std::function<BlockType(int,int,int)>& at){
  // Level 15 travels at most 14 steps with nonzero light. A 15-cell halo
  // therefore computes even the retained one-cell mesh border independently
  // of neighboring chunks' lighting state and update order.
  constexpr int width=SIZE_X+2*LIGHT_RADIUS,depth=SIZE_Z+2*LIGHT_RADIUS;
  constexpr int plane=width*depth;
  const auto sampleIndex=[](int x,int y,int z){return (y*depth+z)*width+x;};
  std::vector<std::uint8_t> light(plane*SIZE_Y,0),attenuation(plane*SIZE_Y,15);
  std::queue<int> open;
  const int ox=m_position.x*SIZE_X,oz=m_position.y*SIZE_Z;
  for(int z=0;z<depth;++z)for(int x=0;x<width;++x){
    int direct=15;
    for(int y=SIZE_Y-1;y>=0;--y){
      const int lx=x-LIGHT_RADIUS,lz=z-LIGHT_RADIUS;
      const auto block=lx>=0&&lx<SIZE_X&&lz>=0&&lz<SIZE_Z?getBlock(lx,y,lz):at(ox+lx,y,oz+lz);
      const int i=sampleIndex(x,y,z);
      attenuation[i]=skyLightAttenuation(block);
      direct=std::max(0,direct-attenuation[i]);
      light[i]=static_cast<std::uint8_t>(direct);
    }
  }
  // Most of the volume is open sky. Queue only the frontier where a seeded
  // cell can actually brighten a neighbor, rather than every sky cell.
  for(int y=0;y<SIZE_Y;++y)for(int z=0;z<depth;++z)for(int x=0;x<width;++x){
    const int i=sampleIndex(x,y,z),next=light[i]-1;
    if(next<=0)continue;
    const auto needsLight=[&](int ni){return light[ni]<next&&attenuation[ni]<15;};
    if((x>0&&needsLight(i-1))||(x+1<width&&needsLight(i+1))||
       (z>0&&needsLight(i-width))||(z+1<depth&&needsLight(i+width))||
       (y>0&&needsLight(i-plane)))open.push(i);
  }
  while(!open.empty()){
    const int i=open.front();open.pop();
    const int x=i%width,z=(i/width)%depth,y=i/plane;
    const int next=light[i]-1;
    if(next<=0)continue;
    for(const auto& d:DIRECTIONS){
      const int nx=x+d[0],ny=y+d[1],nz=z+d[2];
      if(nx<0||nx>=width||ny<0||ny>=SIZE_Y||nz<0||nz>=depth)continue;
      const int ni=sampleIndex(nx,ny,nz);
      if(attenuation[ni]>=15||light[ni]>=next)continue;
      light[ni]=static_cast<std::uint8_t>(next);
      if(next>1)open.push(ni);
    }
  }
  for(int y=0;y<SIZE_Y;++y)for(int z=-1;z<=SIZE_Z;++z)for(int x=-1;x<=SIZE_X;++x)
    m_skyLight[lightIndex(x,y,z)]=light[sampleIndex(x+LIGHT_RADIUS,y,z+LIGHT_RADIUS)];
  m_lightingDirty=false;m_meshDirty=true;
}

void Chunk::generateMesh(const std::function<BlockType(int,int,int)>& at){
  if(m_lightingDirty)computeSkyLight(at);
  std::vector<Vertex> v;std::vector<unsigned> idx;v.reserve(4096);idx.reserve(6144);
  static constexpr int dirs[6][3]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
  static constexpr float q[6][4][3]={
    {{1,0,0},{1,1,0},{1,1,1},{1,0,1}},{{0,0,1},{0,1,1},{0,1,0},{0,0,0}},
    {{0,1,1},{1,1,1},{1,1,0},{0,1,0}},{{0,0,0},{1,0,0},{1,0,1},{0,0,1}},
    {{1,0,1},{1,1,1},{0,1,1},{0,0,1}},{{0,0,0},{0,1,0},{1,1,0},{1,0,0}}};
  const int ox=m_position.x*SIZE_X,oz=m_position.y*SIZE_Z;
  for(int y=0;y<SIZE_Y;++y)for(int z=0;z<SIZE_Z;++z)for(int x=0;x<SIZE_X;++x){
    BlockType type=getBlock(x,y,z);if(type==BlockType::AIR)continue;
    int tile=static_cast<int>(type);if(tile<0||tile>=static_cast<int>(BLOCK_TYPE_COUNT))tile=static_cast<int>(BlockType::BEDROCK);float u0=(tile+.02f)/static_cast<float>(BLOCK_TYPE_COUNT),u1=(tile+.98f)/static_cast<float>(BLOCK_TYPE_COUNT);
    for(int f=0;f<6;++f){int wx=ox+x,wz=oz+z;if(isSolid(at(wx+dirs[f][0],y+dirs[f][1],wz+dirs[f][2])))continue;
      unsigned base=static_cast<unsigned>(v.size());float us[4]={u0,u0,u1,u1},vs[4]={0,1,1,0};
      float light=faceShade(x,y,z,f);for(int n=0;n<4;++n)v.push_back({wx+q[f][n][0],y+q[f][n][1],wz+q[f][n][2],us[n],vs[n],light});
      idx.insert(idx.end(),{base,base+1,base+2,base,base+2,base+3});
    }}
  m_mesh.update(v,idx);m_meshDirty=false;m_ready=true;
}

