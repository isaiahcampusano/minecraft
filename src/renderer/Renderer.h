#pragma once
#include "Texture.h"
#include "ParticleSystem.h"
#include "DropSystem.h"
#include "../core/Shader.h"
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <memory>
class World;
class Player;
class PlayerRenderer;
class Inventory;
class DayNightCycle;
struct RayHit;
class Renderer {
public:
  Renderer(); ~Renderer();
  void spawnBlockBreak(const glm::ivec3& block,BlockType type){m_particles.spawnBlockBreak(block,type);}
  void spawnDrop(const glm::vec3& position,const ItemStack& stack){m_drops.spawn(position,stack);}
  void updateDrops(float dt,const World& world,const glm::vec3& playerPosition,const DropSystem::PickupHandler& tryPickup){m_drops.update(dt,world,playerPosition,tryPickup);}
  void draw(const World&,const Player&,bool showPlayer,float deltaTime,const glm::mat4&,const glm::mat4&,const glm::vec3& camera,const RayHit&,int width,int height,const std::string& hud,const Inventory&,bool inventoryOpen,bool tableOpen,int creativePage,const DayNightCycle&);
private:
  Shader m_shader,m_colorShader,m_skyShader; Texture m_texture; ParticleSystem m_particles; DropSystem m_drops;
  GLuint m_lineVao=0,m_lineVbo=0,m_skyVao=0,m_skyVbo=0;
  std::unique_ptr<PlayerRenderer> m_playerRenderer;
  void drawSky(const DayNightCycle&); void drawOutline(const RayHit&,const glm::mat4&,const glm::mat4&); void drawMiningCrack(const glm::ivec3&,float,const glm::mat4&,const glm::mat4&); void drawOverlay(int,int,const std::string&,const Player&,const Inventory&,bool inventoryOpen,bool tableOpen,int creativePage);
};
