#pragma once
#include "../core/Shader.h"
#include "../player/PlayerAnimator.h"
#include "../player/Item.h"
#include <glm/mat4x4.hpp>
#include <memory>
class CuboidMesh;class Player;class DayNightCycle;

class PlayerRenderer{
public:PlayerRenderer();~PlayerRenderer();void draw(const Player&,float dt,const glm::mat4&,const glm::mat4&,const glm::vec3& camera,const DayNightCycle&);
  void drawFirstPerson(const Player&,const ItemStack&,float dt,bool mining,float useSwingTimer,const glm::mat4&,const glm::mat4&,const glm::vec3& camera,const DayNightCycle&);
private:
  Shader m_shader;PlayerAnimator m_animator;
  std::unique_ptr<CuboidMesh> m_head,m_torso,m_arm,m_leg,m_eye,m_mouth;
  std::unique_ptr<CuboidMesh> m_heldItem;
  float m_bodyYaw=0,m_firstPersonTime=0;bool m_hasBodyYaw=false;
  void drawPart(const CuboidMesh&,const glm::mat4&);
};
