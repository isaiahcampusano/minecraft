#pragma once
#include "Texture.h"
#include "../core/Shader.h"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <memory>
class World;
class Player;
class PlayerRenderer;
struct RayHit;
class Renderer {
public:
  Renderer(); ~Renderer();
  void draw(const World&,const Player&,bool showPlayer,const glm::mat4&,const glm::mat4&,const glm::vec3& camera,const RayHit&,int width,int height,const std::string& hud);
private:
  Shader m_shader,m_colorShader,m_skyShader; Texture m_texture;
  GLuint m_lineVao=0,m_lineVbo=0,m_skyVao=0,m_skyVbo=0;
  std::unique_ptr<PlayerRenderer> m_playerRenderer;
  void drawSky(); void drawOutline(const RayHit&,const glm::mat4&,const glm::mat4&); void drawOverlay(int,int,const std::string&);
};
