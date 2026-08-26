#pragma once
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <array>

class CuboidMesh{
public:
  CuboidMesh(glm::vec3 dimensions,const std::array<glm::vec3,6>& faceColors);
  ~CuboidMesh(); CuboidMesh(const CuboidMesh&)=delete; CuboidMesh& operator=(const CuboidMesh&)=delete;
  void draw()const;
private:
  GLuint m_vao=0,m_vbo=0,m_ebo=0;GLsizei m_indexCount=0;
};

