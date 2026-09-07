#pragma once
#include <glm/glm.hpp>
#include "../world/Block.h"
#include "../world/BlockProperties.h"
#include "SurvivalState.h"
#include <algorithm>
class World;

class Player {
public:
  static constexpr float WIDTH=.6f,HEIGHT=1.8f,EYE_HEIGHT=1.62f,FULL_AUTO_BREAK_COOLDOWN=.12f,MINING_FIXED_STEP=1.f/120.f,MAX_MINING_FRAME_TIME=.05f;
  explicit Player(glm::vec3 spawn={500.f,8.f,500.f}):position(spawn){}
  glm::vec3 position,velocity{0}; float yaw=-90.f,pitch=-15.f; bool onGround=false,isFlying=false;SurvivalState survival;
  glm::ivec3 targetedBlock{-1,-1,-1}; float blockBreakProgress=0.f,blockBreakCooldown=0.f,blockBreakAccumulator=0.f;
  glm::vec3 forward()const; glm::vec3 right()const; glm::vec3 eyePosition()const{return position+glm::vec3(0,EYE_HEIGHT,0);}
  void look(float dx,float dy); void update(float dt,const World&); bool jump(bool sprinting=false); void toggleFly();void respawn(const glm::vec3& spawn);
  bool overlapsBlock(const glm::ivec3&)const;
  void setMiningTarget(const glm::ivec3& block){if(targetedBlock!=block){targetedBlock=block;resetMiningProgress();}}
  void resetMiningProgress(){blockBreakProgress=0.f;blockBreakAccumulator=0.f;}
  void startMiningCooldown(){blockBreakCooldown=FULL_AUTO_BREAK_COOLDOWN;}
  void resetMiningCooldown(){blockBreakCooldown=0.f;}
  void clearMiningTarget(){targetedBlock={-1,-1,-1};resetMiningProgress();}
  bool advanceMining(BlockType type,float dt,const ItemStack& held={}){const auto& properties=getBlockProperties(type);if(!properties.diggable||properties.hardness<=0.f){resetMiningProgress();return false;}blockBreakAccumulator+=std::clamp(dt,0.f,MAX_MINING_FRAME_TIME);while(blockBreakAccumulator+1e-6f>=MINING_FIXED_STEP){blockBreakAccumulator=std::max(0.f,blockBreakAccumulator-MINING_FIXED_STEP);float miningTime=MINING_FIXED_STEP;const float cooldownTime=std::min(blockBreakCooldown,miningTime);blockBreakCooldown-=cooldownTime;miningTime-=cooldownTime;blockBreakProgress=std::min(1.f,blockBreakProgress+miningTime*miningSpeedMultiplier(held,properties)/properties.hardness);if(blockBreakProgress>=1.f)return true;}return false;}
};

