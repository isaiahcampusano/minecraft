#pragma once
#include "Texture.h"
#include "../core/Shader.h"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <string>
class World;
struct RayHit;
class Renderer {
public:
  Renderer(); ~Renderer();
  void draw(const World&,const glm::mat4&,const glm::mat4&,const glm::vec3& camera,const RayHit&,int width,int height,const std::string& hud);
private:
  Shader m_shader,m_colorShader,m_skyShader; Texture m_texture;
  GLuint m_lineVao=0,m_lineVbo=0,m_skyVao=0,m_skyVbo=0;
  void drawSky(); void drawOutline(const RayHit&,const glm::mat4&,const glm::mat4&); void drawOverlay(int,int,const std::string&);
};
