#include "CuboidMesh.h"
#include <vector>
#include <cstddef>
#include <stdexcept>

namespace{struct Vertex{glm::vec3 position,normal,color;};}
CuboidMesh::CuboidMesh(glm::vec3 d,const std::array<glm::vec3,6>& colors){
  const glm::vec3 h=d*.5f;
  const glm::vec3 corners[8]={{-h.x,-h.y,-h.z},{h.x,-h.y,-h.z},{h.x,-h.y,h.z},{-h.x,-h.y,h.z},{-h.x,h.y,-h.z},{h.x,h.y,-h.z},{h.x,h.y,h.z},{-h.x,h.y,h.z}};
  const int faces[6][4]={{1,5,6,2},{0,3,7,4},{4,7,6,5},{0,1,2,3},{2,6,7,3},{0,4,5,1}};
  const glm::vec3 normals[6]={{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}};
  std::vector<Vertex> vertices;std::vector<unsigned> indices;vertices.reserve(24);indices.reserve(36);
  for(int f=0;f<6;++f){if(glm::dot(glm::cross(corners[faces[f][1]]-corners[faces[f][0]],corners[faces[f][2]]-corners[faces[f][0]]),normals[f])<=0)throw std::logic_error("Cuboid face winding is not outward");unsigned base=static_cast<unsigned>(vertices.size());for(int n=0;n<4;++n)vertices.push_back({corners[faces[f][n]],normals[f],colors[f]});indices.insert(indices.end(),{base,base+1,base+2,base,base+2,base+3});}
  m_indexCount=static_cast<GLsizei>(indices.size());glGenVertexArrays(1,&m_vao);glGenBuffers(1,&m_vbo);glGenBuffers(1,&m_ebo);glBindVertexArray(m_vao);glBindBuffer(GL_ARRAY_BUFFER,m_vbo);glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(Vertex)),vertices.data(),GL_STATIC_DRAW);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,m_ebo);glBufferData(GL_ELEMENT_ARRAY_BUFFER,static_cast<GLsizeiptr>(indices.size()*sizeof(unsigned)),indices.data(),GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,position)));glEnableVertexAttribArray(0);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,normal)));glEnableVertexAttribArray(1);glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),reinterpret_cast<void*>(offsetof(Vertex,color)));glEnableVertexAttribArray(2);glBindVertexArray(0);
}
CuboidMesh::~CuboidMesh(){if(m_ebo)glDeleteBuffers(1,&m_ebo);if(m_vbo)glDeleteBuffers(1,&m_vbo);if(m_vao)glDeleteVertexArrays(1,&m_vao);}
void CuboidMesh::draw()const{glBindVertexArray(m_vao);glDrawElements(GL_TRIANGLES,m_indexCount,GL_UNSIGNED_INT,nullptr);}
