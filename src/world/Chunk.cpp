#include "Chunk.h"
#include <algorithm>
#include <queue>

Chunk::Chunk(glm::ivec2 p):m_position(p){m_blocks.fill(BlockType::AIR);}
void Chunk::setBlock(int x,int y,int z,BlockType t){if(x>=0&&x<SIZE_X&&y>=0&&y<SIZE_Y&&z>=0&&z<SIZE_Z){m_blocks[index(x,y,z)]=t;m_dirty=true;}}
BlockType Chunk::getBlock(int x,int y,int z)const{return x>=0&&x<SIZE_X&&y>=0&&y<SIZE_Y&&z>=0&&z<SIZE_Z?m_blocks[index(x,y,z)]:BlockType::AIR;}
std::uint8_t Chunk::skyLight(int x,int y,int z)const{return x>=0&&x<SIZE_X&&y>=0&&y<SIZE_Y&&z>=0&&z<SIZE_Z?m_skyLight[index(x,y,z)]:0;}

void Chunk::computeSkyLight(const std::function<BlockType(int,int,int)>& at){
  // Full per-chunk recomputation can be briefly inconsistent at chunk seams after
  // boundary edits until the neighboring dirty chunk also recomputes. A fully
  // incremental cross-chunk removal/propagation engine is intentionally out of scope.
  m_skyLight.fill(0);std::queue<glm::ivec3> open;
  for(int z=0;z<SIZE_Z;++z)for(int x=0;x<SIZE_X;++x)for(int y=SIZE_Y-1;y>=0;--y){if(isSolid(getBlock(x,y,z)))break;m_skyLight[index(x,y,z)]=15;open.push({x,y,z});}
  static constexpr int dirs[6][3]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
  const int ox=m_position.x*SIZE_X,oz=m_position.y*SIZE_Z;
  while(!open.empty()){glm::ivec3 p=open.front();open.pop();std::uint8_t light=skyLight(p.x,p.y,p.z);if(light<=1)continue;for(const auto& d:dirs){int nx=p.x+d[0],ny=p.y+d[1],nz=p.z+d[2];if(ny<0||ny>=SIZE_Y)continue;if(isSolid(at(ox+nx,ny,oz+nz)))continue;if(nx<0||nx>=SIZE_X||nz<0||nz>=SIZE_Z)continue;std::uint8_t next=static_cast<std::uint8_t>(light-1);if(skyLight(nx,ny,nz)<next){m_skyLight[index(nx,ny,nz)]=next;open.push({nx,ny,nz});}}}
  // Solids retain the skylight incident on their top surface for mesh shading.
  for(int z=0;z<SIZE_Z;++z)for(int x=0;x<SIZE_X;++x)for(int y=0;y<SIZE_Y;++y)if(isSolid(getBlock(x,y,z)))m_skyLight[index(x,y,z)]=y+1<SIZE_Y&&getBlock(x,y+1,z)==BlockType::AIR?skyLight(x,y+1,z):0;
}

void Chunk::generateMesh(const std::function<BlockType(int,int,int)>& at){
  computeSkyLight(at);
  std::vector<Vertex> v;std::vector<unsigned> idx;v.reserve(4096);idx.reserve(6144);
  static constexpr int dirs[6][3]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
  static constexpr float shade[6]={.82f,.72f,1.f,.55f,.9f,.68f};
  static constexpr float q[6][4][3]={
    {{1,0,0},{1,1,0},{1,1,1},{1,0,1}},{{0,0,1},{0,1,1},{0,1,0},{0,0,0}},
    {{0,1,1},{1,1,1},{1,1,0},{0,1,0}},{{0,0,0},{1,0,0},{1,0,1},{0,0,1}},
    {{1,0,1},{1,1,1},{0,1,1},{0,0,1}},{{0,0,0},{0,1,0},{1,1,0},{1,0,0}}};
  const int ox=m_position.x*SIZE_X,oz=m_position.y*SIZE_Z;
  for(int y=0;y<SIZE_Y;++y)for(int z=0;z<SIZE_Z;++z)for(int x=0;x<SIZE_X;++x){
    BlockType type=getBlock(x,y,z);if(type==BlockType::AIR)continue;
    int tile=static_cast<int>(type);if(tile<0||tile>=10)tile=3;float u0=(tile+.02f)/10.f,u1=(tile+.98f)/10.f;
    for(int f=0;f<6;++f){int wx=ox+x,wz=oz+z;if(isSolid(at(wx+dirs[f][0],y+dirs[f][1],wz+dirs[f][2])))continue;
      unsigned base=static_cast<unsigned>(v.size());float us[4]={u0,u0,u1,u1},vs[4]={0,1,1,0};
      float light=std::max(.05f,skyLight(x,y,z)/15.f);for(int n=0;n<4;++n)v.push_back({wx+q[f][n][0],y+q[f][n][1],wz+q[f][n][2],us[n],vs[n],shade[f]*light});
      idx.insert(idx.end(),{base,base+1,base+2,base,base+2,base+3});
    }}
  m_mesh.update(v,idx);m_dirty=false;
}

