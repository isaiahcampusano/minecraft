#include "PlayerAnimator.h"
#include "Player.h"
#include <algorithm>
#include <cmath>

void PlayerAnimator::approach(float& current,float target,float dt){float factor=1.f-std::exp(-12.f*dt);current+=(target-current)*factor;}
void PlayerAnimator::update(const Player&p,float dt,float bodyYaw){float speed=std::sqrt(p.velocity.x*p.velocity.x+p.velocity.z*p.velocity.z);bool moving=p.onGround&&!p.isFlying&&speed>.1f;bool sprinting=speed>7.f;m_idleTime+=dt;m_target={};float cameraYaw=glm::radians(-p.yaw-90.f);float yawDelta=std::atan2(std::sin(cameraYaw-bodyYaw),std::cos(cameraYaw-bodyYaw));m_target.headYaw=std::clamp(yawDelta,-1.4f,1.4f);m_target.headPitch=std::clamp(-glm::radians(p.pitch),-.75f,.75f);
  if(moving){m_cycle+=dt*(sprinting?13.f:8.f);float swing=std::sin(m_cycle),amplitude=sprinting?1.25f:1.f;m_target.leftArm=swing*amplitude;m_target.rightArm=-swing*amplitude;m_target.leftLeg=-swing*amplitude*.8f;m_target.rightLeg=swing*amplitude*.8f;m_target.leftArmOut=-.1f-swing*.1f;m_target.rightArmOut=.1f-swing*.1f;m_target.torsoBob=std::abs(std::sin(m_cycle*2.f))*(sprinting?.075f:.055f);m_target.torsoLean=sprinting?.18f:.05f;m_target.torsoSway=swing*(sprinting?.075f:.045f);}
  else{m_target.torsoBob=std::sin(m_idleTime*2.f)*.012f;if(p.onGround)m_cycle=0;}
  if(!p.onGround&&!p.isFlying){if(p.velocity.y>0){m_target.leftArm=-1.15f;m_target.rightArm=-1.15f;m_target.leftArmOut=-.18f;m_target.rightArmOut=.18f;m_target.leftLeg=.28f;m_target.rightLeg=-.28f;}else{m_target.leftArm=.38f;m_target.rightArm=-.38f;m_target.leftArmOut=-.35f;m_target.rightArmOut=.35f;m_target.leftLeg=.2f;m_target.rightLeg=-.2f;}}
  if(p.isFlying){m_target.torsoLean=.65f;m_target.leftArm=-.55f;m_target.rightArm=-.55f;m_target.leftLeg=.18f;m_target.rightLeg=-.18f;}
  approach(m_current.headYaw,m_target.headYaw,dt);approach(m_current.headPitch,m_target.headPitch,dt);approach(m_current.torsoBob,m_target.torsoBob,dt);approach(m_current.torsoLean,m_target.torsoLean,dt);approach(m_current.torsoSway,m_target.torsoSway,dt);
  approach(m_current.leftArm,m_target.leftArm,dt);approach(m_current.rightArm,m_target.rightArm,dt);approach(m_current.leftArmOut,m_target.leftArmOut,dt);approach(m_current.rightArmOut,m_target.rightArmOut,dt);approach(m_current.leftLeg,m_target.leftLeg,dt);approach(m_current.rightLeg,m_target.rightLeg,dt);
}
