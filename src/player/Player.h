#pragma once
#include <glm/glm.hpp>
#include "../world/Block.h"
#include "../world/BlockProperties.h"
#include <algorithm>
class World;

class Player {
public:
  static constexpr float WIDTH=.6f,HEIGHT=1.8f,EYE_HEIGHT=1.62f;
  explicit Player(glm::vec3 spawn={500.f,8.f,500.f}):position(spawn){}
  glm::vec3 position,velocity{0}; float yaw=-90.f,pitch=-15.f; bool onGround=false,isFlying=false;
  glm::ivec3 targetedBlock{-1,-1,-1}; float blockBreakProgress=0.f;
  glm::vec3 forward()const; glm::vec3 right()const; glm::vec3 eyePosition()const{return position+glm::vec3(0,EYE_HEIGHT,0);}
  void look(float dx,float dy); void update(float dt,const World&); void jump(); void toggleFly();
  bool overlapsBlock(const glm::ivec3&)const;
  void setMiningTarget(const glm::ivec3& block){if(targetedBlock!=block){targetedBlock=block;blockBreakProgress=0.f;}}
  void resetMiningProgress(){blockBreakProgress=0.f;}
  void clearMiningTarget(){targetedBlock={-1,-1,-1};blockBreakProgress=0.f;}
  bool advanceMining(BlockType type,float dt){const auto& properties=getBlockProperties(type);if(!properties.diggable||properties.hardness<=0.f){blockBreakProgress=0.f;return false;}blockBreakProgress=std::min(1.f,blockBreakProgress+std::max(dt,0.f)/properties.hardness);return blockBreakProgress>=1.f;}
};

