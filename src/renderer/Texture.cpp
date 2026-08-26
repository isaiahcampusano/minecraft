#include "Texture.h"
Texture::Texture(){
  const unsigned char pixels[]={110,180,70,255, 125,76,42,255, 55,55,55,255, 90,150,55,255};
  glGenTextures(1,&m_id);glBindTexture(GL_TEXTURE_2D,m_id);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,4,1,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
}
Texture::~Texture(){if(m_id)glDeleteTextures(1,&m_id);} void Texture::bind()const{glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,m_id);}

