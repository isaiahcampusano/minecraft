#include "Texture.h"
#include "../world/BlockRegistry.h"
#include <array>
Texture::Texture(){
  constexpr int tileSize=16,tileCount=static_cast<int>(BLOCK_TYPE_COUNT),width=tileSize*tileCount,height=tileSize;
  std::array<unsigned char,width*height*4> pixels{};
  for(int tile=0;tile<tileCount;++tile)for(int y=0;y<tileSize;++y)for(int x=0;x<tileSize;++x){
    const bool edge=x==0||y==0||x==tileSize-1||y==tileSize-1;
    int noise=((x*17+y*31+tile*13)%17)-8;
    if(tile==5||tile==8)noise=((x*29+y*43+tile*7)%31)-15;
    const int factor=edge?-22:noise;
    const int i=(y*width+tile*tileSize+x)*4;
    const BlockColor color=BLOCK_COLORS[static_cast<std::size_t>(tile)];const int channels[3]={color.r,color.g,color.b};
    for(int c=0;c<3;++c)pixels[i+c]=static_cast<unsigned char>(channels[c]+factor);
    pixels[i+3]=255;
  }
  glGenTextures(1,&m_id);glBindTexture(GL_TEXTURE_2D,m_id);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels.data());
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
}
Texture::~Texture(){if(m_id)glDeleteTextures(1,&m_id);} void Texture::bind()const{glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,m_id);}
