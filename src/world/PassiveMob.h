#pragma once
#include <cstddef>
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

using MobId=std::uint64_t;

inline constexpr float MOB_STEP_HEIGHT=1.0f;

enum class MobType:std::uint8_t{COW,PIG,SHEEP,COUNT};
enum class MobAIState:std::uint8_t{IDLE,WANDERING,FOLLOWING_PARENT,GRAZING,FLEEING};

struct MobHit{bool hit=false;MobId id=0;float distance=0;};

struct MobData{
  MobId id=0;MobType type=MobType::COW;
  float x=0,y=0,z=0,yaw=0;
  std::int32_t health=0;
  float age=0,grazeCooldown=0;
};

struct PassiveMob{
  MobId id=0;MobType type=MobType::COW;MobAIState state=MobAIState::IDLE;
  glm::vec3 position{0},velocity{0},target{0},fleeSource{0};
  float yaw=0,age=0,stateTimer=0,aiTimer=0,hurtTimer=0,grazeCooldown=0,animationTime=0;
  int health=0;bool onGround=false;
  std::vector<glm::ivec3> path;std::size_t pathIndex=0;
  constexpr bool isBaby()const{return age<0;}
};

inline constexpr int mobMaxHealth(MobType type){return type==MobType::SHEEP?8:10;}
inline constexpr float mobWidth(MobType){return .9f;}
inline constexpr float mobHeight(MobType type){return type==MobType::PIG?.9f:(type==MobType::SHEEP?1.3f:1.4f);}
inline float mobScale(const PassiveMob& mob){return mob.isBaby()?.5f:1.f;}
inline constexpr const char* mobName(MobType type){return type==MobType::COW?"COW":(type==MobType::PIG?"PIG":(type==MobType::SHEEP?"SHEEP":"UNKNOWN"));}
