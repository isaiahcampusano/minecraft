#include "Chunk.h"
#include <algorithm>

Chunk::Chunk(glm::ivec2 p):m_position(p){m_blocks.fill(BlockType::AIR);}
void Chunk::setBlock(int x,int y,int z,BlockType t){if(x>=0&&x<SIZE_X&&y>=0&&y<SIZE_Y&&z>=0&&z<SIZE_Z){m_blocks[index(x,y,z)]=t;m_dirty=true;}}
BlockType Chunk::getBlock(int x,int y,int z)const{return x>=0&&x<SIZE_X&&y>=0&&y<SIZE_Y&&z>=0&&z<SIZE_Z?m_blocks[index(x,y,z)]:BlockType::AIR;}

void Chunk::generateMesh(const std::function<BlockType(int,int,int)>& at){
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
    int tile=static_cast<int>(type)-1; float u0=(tile+.02f)/9.f,u1=(tile+.98f)/9.f;
    for(int f=0;f<6;++f){int wx=ox+x,wz=oz+z;if(isSolid(at(wx+dirs[f][0],y+dirs[f][1],wz+dirs[f][2])))continue;
      unsigned base=static_cast<unsigned>(v.size());float us[4]={u0,u0,u1,u1},vs[4]={0,1,1,0};
      for(int n=0;n<4;++n)v.push_back({wx+q[f][n][0],y+q[f][n][1],wz+q[f][n][2],us[n],vs[n],shade[f]});
      idx.insert(idx.end(),{base,base+1,base+2,base,base+2,base+3});
    }}
  m_mesh.update(v,idx);m_dirty=false;
}

