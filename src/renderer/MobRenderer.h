#pragma once
#include "../core/Shader.h"
#include <glm/mat4x4.hpp>
#include <memory>

class CuboidMesh;
class DayNightCycle;
class PassiveMobSystem;
class World;

class MobRenderer{
public:
  MobRenderer();~MobRenderer();
  void draw(const PassiveMobSystem& mobs,const World& world,const glm::mat4& view,const glm::mat4& projection,const glm::vec3& camera,const DayNightCycle& dayNight);
private:
  Shader m_shader;
  std::unique_ptr<CuboidMesh> m_body,m_wool,m_head,m_leg,m_snout;
  void drawPart(const CuboidMesh& mesh,const glm::mat4& model,const glm::vec3& color,float hurt);
};
