#pragma once
#include "../core/Shader.h"
#include "../player/PlayerAnimator.h"
#include <glm/mat4x4.hpp>
#include <memory>
class CuboidMesh;class Player;

class PlayerRenderer{
public:PlayerRenderer();~PlayerRenderer();void draw(const Player&,float dt,const glm::mat4&,const glm::mat4&,const glm::vec3& camera);
private:
  Shader m_shader;PlayerAnimator m_animator;
  std::unique_ptr<CuboidMesh> m_head,m_torso,m_arm,m_leg,m_eye,m_mouth;
  void drawPart(const CuboidMesh&,const glm::mat4&);
};

