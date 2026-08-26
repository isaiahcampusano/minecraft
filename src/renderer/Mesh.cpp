#include "Mesh.h"
#include <cstddef>

Mesh::~Mesh(){ if(m_ebo)glDeleteBuffers(1,&m_ebo); if(m_vbo)glDeleteBuffers(1,&m_vbo); if(m_vao)glDeleteVertexArrays(1,&m_vao); }
void Mesh::update(const std::vector<Vertex>& v,const std::vector<unsigned>& i){
  if(!m_vao){glGenVertexArrays(1,&m_vao);glGenBuffers(1,&m_vbo);glGenBuffers(1,&m_ebo);}
  glBindVertexArray(m_vao); glBindBuffer(GL_ARRAY_BUFFER,m_vbo); glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(v.size()*sizeof(Vertex)),v.data(),GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ebo); glBufferData(GL_ELEMENT_ARRAY_BUFFER,static_cast<GLsizeiptr>(i.size()*sizeof(unsigned)),i.data(),GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,x))); glEnableVertexAttribArray(0);
  glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,u))); glEnableVertexAttribArray(1);
  glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,shade))); glEnableVertexAttribArray(2);
  glBindVertexArray(0); m_count=static_cast<GLsizei>(i.size());
}
void Mesh::render()const{if(!m_count)return;glBindVertexArray(m_vao);glDrawElements(GL_TRIANGLES,m_count,GL_UNSIGNED_INT,nullptr);}

