#include "PassiveMobSystem.h"
#include "LootTable.h"
#include "Pathfinder.h"
#include "World.h"
#include "../player/ToolRegistry.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <unordered_set>
#include <utility>

namespace{
constexpr float BABY_AGE=-1200.f;
float horizontalDistance(const glm::vec3& a,const glm::vec3& b){return glm::length(glm::vec2{a.x-b.x,a.z-b.z});}
bool mobCollides(const PassiveMob& mob,const glm::vec3& position,const World& world){const float scale=mobScale(mob),halfWidth=mobWidth(mob.type)*scale*.5f,height=mobHeight(mob.type)*scale,e=.0001f;const glm::vec3 lo=position+glm::vec3(-halfWidth,0,-halfWidth),hi=position+glm::vec3(halfWidth,height,halfWidth);for(int y=static_cast<int>(std::floor(lo.y));y<=static_cast<int>(std::floor(hi.y-e));++y)for(int z=static_cast<int>(std::floor(lo.z));z<=static_cast<int>(std::floor(hi.z-e));++z)for(int x=static_cast<int>(std::floor(lo.x));x<=static_cast<int>(std::floor(hi.x-e));++x)if(isSolid(world.getBlock(x,y,z)))return true;return false;}
// Clip an axis movement against every voxel crossed by the full mob box.
// Endpoint-only checks can miss a ceiling when a small mob steps upward.
float sweepMob(const PassiveMob& mob,glm::vec3& position,int axis,float distance,const World& world){
  constexpr float epsilon=.0001f;
  if(distance==0.f)return 0.f;
  const float halfWidth=mobWidth(mob.type)*mobScale(mob)*.5f;
  const glm::vec3 lo=position+glm::vec3(-halfWidth,0,-halfWidth);
  const glm::vec3 hi=position+glm::vec3(halfWidth,mobHeight(mob.type)*mobScale(mob),halfWidth);
  glm::vec3 sweptLo=lo,sweptHi=hi;
  sweptLo[axis]+=std::min(0.f,distance);sweptHi[axis]+=std::max(0.f,distance);
  float allowed=distance;
  for(int y=static_cast<int>(std::floor(sweptLo.y+epsilon));y<=static_cast<int>(std::floor(sweptHi.y-epsilon));++y)
    for(int z=static_cast<int>(std::floor(sweptLo.z+epsilon));z<=static_cast<int>(std::floor(sweptHi.z-epsilon));++z)
      for(int x=static_cast<int>(std::floor(sweptLo.x+epsilon));x<=static_cast<int>(std::floor(sweptHi.x-epsilon));++x){
        if(!isSolid(world.getBlock(x,y,z)))continue;
        const glm::vec3 blockLo{x,y,z},blockHi=blockLo+glm::vec3(1.f);
        if(distance>0.f&&blockLo[axis]>=hi[axis]-epsilon)
          allowed=std::min(allowed,std::max(0.f,blockLo[axis]-hi[axis]));
        else if(distance<0.f&&blockHi[axis]<=lo[axis]+epsilon)
          allowed=std::max(allowed,std::min(0.f,blockHi[axis]-lo[axis]));
      }
  position[axis]+=allowed;
  return allowed;
}

bool rayBox(const glm::vec3& origin,const glm::vec3& direction,const glm::vec3& low,const glm::vec3& high,float maxDistance,float& distance){float nearValue=0,farValue=maxDistance;for(int axis=0;axis<3;++axis){if(std::abs(direction[axis])<1e-6f){if(origin[axis]<low[axis]||origin[axis]>high[axis])return false;continue;}float first=(low[axis]-origin[axis])/direction[axis],second=(high[axis]-origin[axis])/direction[axis];if(first>second)std::swap(first,second);nearValue=std::max(nearValue,first);farValue=std::min(farValue,second);if(nearValue>farValue)return false;}distance=nearValue;return nearValue<=maxDistance&&farValue>=0;}
}

std::uint32_t PassiveMobSystem::random(){m_randomState^=m_randomState<<13;m_randomState^=m_randomState>>17;m_randomState^=m_randomState<<5;return m_randomState;}
float PassiveMobSystem::random01(){return(random()&0x00ffffffu)/static_cast<float>(0x01000000u);}
int PassiveMobSystem::randomInt(int minimum,int maximum){return minimum+static_cast<int>(random()%static_cast<std::uint32_t>(maximum-minimum+1));}

PassiveMob& PassiveMobSystem::addMob(MobType type,const glm::vec3& position,bool baby){PassiveMob mob;mob.id=m_nextId++;mob.type=type;mob.position=position;mob.health=mobMaxHealth(type);mob.age=baby?BABY_AGE:0;mob.stateTimer=2.f+random01()*3.f;mob.aiTimer=random01()*AI_STEP;mob.grazeCooldown=5.f+random01()*10.f;m_mobs.push_back(mob);return m_mobs.back();}
void PassiveMobSystem::clear(){m_mobs.clear();m_nextId=1;m_tickAccumulator=m_spawnTimer=m_grassTimer=0;m_initialized=false;}
std::size_t PassiveMobSystem::nearbyCount(const glm::vec3& position,float radius)const{return static_cast<std::size_t>(std::count_if(m_mobs.begin(),m_mobs.end(),[&](const PassiveMob& mob){return glm::distance(mob.position,position)<=radius;}));}
int PassiveMobSystem::surfaceY(const World& world,int x,int z)const{if(x<0||x>=1000||z<0||z>=1000||!world.isChunkLoadedAt(x,z))return-1;for(int y=254;y>=0;--y)if(isSolid(world.getBlock(x,y,z)))return y;return-1;}

bool PassiveMobSystem::canSpawnAt(const World& world,const glm::ivec3& feet,float daylight)const{
  if(daylight<.45f||feet.x<1||feet.x>=999||feet.z<1||feet.z>=999||feet.y<=0||feet.y>=254||!world.isChunkLoadedAt(feet.x,feet.z))return false;
  const BlockType ground=world.getBlock(feet.x,feet.y-1,feet.z);if(ground!=BlockType::GRASS&&ground!=BlockType::DIRT)return false;
  if(isSolid(world.getBlock(feet.x,feet.y,feet.z))||isSolid(world.getBlock(feet.x,feet.y+1,feet.z))||world.skyLight(feet.x,feet.y,feet.z)<9)return false;
  const int chunkX=feet.x/16,chunkZ=feet.z/16;int inChunk=0,cluster=0;for(const auto& mob:m_mobs){if(static_cast<int>(std::floor(mob.position.x))/16==chunkX&&static_cast<int>(std::floor(mob.position.z))/16==chunkZ)++inChunk;if(horizontalDistance(mob.position,glm::vec3(feet))<8.f)++cluster;}return inChunk<4&&cluster<4;
}

bool PassiveMobSystem::findSpawnPosition(World& world,const glm::vec3& playerPosition,float daylight,glm::ivec3& feet){for(int attempt=0;attempt<96;++attempt){const int dx=randomInt(-48,48),dz=randomInt(-48,48),distanceSquared=dx*dx+dz*dz;if(distanceSquared<16*16||distanceSquared>48*48)continue;const int x=std::clamp(static_cast<int>(std::floor(playerPosition.x))+dx,1,998),z=std::clamp(static_cast<int>(std::floor(playerPosition.z))+dz,1,998),groundY=surfaceY(world,x,z);if(groundY<0)continue;glm::ivec3 candidate{x,groundY+1,z};if(canSpawnAt(world,candidate,daylight)){feet=candidate;return true;}}return false;}

bool PassiveMobSystem::spawnPack(MobType type,World& world,const glm::vec3& playerPosition,float daylight,std::size_t limit){if(m_mobs.size()>=GLOBAL_CAP||nearbyCount(playerPosition)>=LOCAL_CAP||limit==0)return false;glm::ivec3 base;if(!findSpawnPosition(world,playerPosition,daylight,base))return false;const int requested=randomInt(1,4);int spawned=0;for(int member=0;member<requested&&static_cast<std::size_t>(spawned)<limit&&m_mobs.size()<GLOBAL_CAP&&nearbyCount(playerPosition)<LOCAL_CAP;++member){glm::ivec3 feet=base;if(member>0){feet.x+=randomInt(-3,3);feet.z+=randomInt(-3,3);const glm::vec2 offset{feet.x+.5f-playerPosition.x,feet.z+.5f-playerPosition.z};const float distanceSquared=glm::dot(offset,offset);if(distanceSquared<16.f*16.f||distanceSquared>48.f*48.f)continue;const int groundY=surfaceY(world,feet.x,feet.z);if(groundY<0)continue;feet.y=groundY+1;if(!canSpawnAt(world,feet,daylight))continue;}addMob(type,glm::vec3(feet.x+.5f,static_cast<float>(feet.y),feet.z+.5f),member>0&&randomInt(0,99)<15);++spawned;}return spawned>0;}

void PassiveMobSystem::initialize(World& world,const glm::vec3& playerPosition,float daylight){if(m_initialized||daylight<.45f)return;std::array<bool,static_cast<std::size_t>(MobType::COUNT)> present{};for(const auto& mob:m_mobs)present[static_cast<std::size_t>(mob.type)]=true;for(int type=0;type<static_cast<int>(MobType::COUNT);++type)if(!present[static_cast<std::size_t>(type)])for(int attempt=0;attempt<8&&!spawnPack(static_cast<MobType>(type),world,playerPosition,daylight,1);++attempt){}present.fill(false);for(const auto& mob:m_mobs)present[static_cast<std::size_t>(mob.type)]=true;if(std::any_of(present.begin(),present.end(),[](bool value){return!value;}))return;while(m_mobs.size()<TARGET_NEARBY){const auto before=m_mobs.size();spawnPack(static_cast<MobType>(randomInt(0,static_cast<int>(MobType::COUNT)-1)),world,playerPosition,daylight,TARGET_NEARBY-m_mobs.size());if(m_mobs.size()==before)break;}m_initialized=true;}

bool PassiveMobSystem::active(const PassiveMob& mob,const World& world,const glm::vec3& playerPosition)const{return world.isChunkLoadedAt(static_cast<int>(std::floor(mob.position.x)),static_cast<int>(std::floor(mob.position.z)))&&glm::distance(mob.position,playerPosition)<=ACTIVE_DISTANCE;}

bool PassiveMobSystem::findWanderDestination(const PassiveMob& mob,const World& world,const glm::ivec3& intended,glm::ivec3& target,std::vector<glm::ivec3>& path)const{
  const glm::ivec3 start=glm::ivec3(glm::floor(mob.position));
  const int startY=start.y;
  for(int radius=0;radius<=2;++radius)for(int dz=-radius;dz<=radius;++dz)for(int dx=-radius;dx<=radius;++dx){
    if(std::max(std::abs(dx),std::abs(dz))!=radius)continue;
    const int x=std::clamp(intended.x+dx,1,998),z=std::clamp(intended.z+dz,1,998);
    if(!world.isChunkLoadedAt(x,z))continue;
    for(int y=startY+2;y>=startY-3;--y){
      const glm::ivec3 candidate{x,y,z};
      if(!Pathfinder::isWalkable(world,candidate))continue;
      auto candidatePath=Pathfinder::findPath(world,start,candidate);
      if(candidatePath.empty()&&candidate!=start)continue;
      target=candidate;path=std::move(candidatePath);return true;
    }
  }
  return false;
}

void PassiveMobSystem::chooseWander(PassiveMob& mob,const World& world){const float angle=random01()*6.2831853f,distance=6.f+random01()*6.f;const int x=std::clamp(static_cast<int>(std::floor(mob.position.x+std::cos(angle)*distance)),1,998),z=std::clamp(static_cast<int>(std::floor(mob.position.z+std::sin(angle)*distance)),1,998);glm::ivec3 feet;std::vector<glm::ivec3> path;if(!findWanderDestination(mob,world,{x,0,z},feet,path)){mob.state=MobAIState::IDLE;mob.stateTimer=2.f+random01()*3.f;mob.path.clear();mob.pathIndex=0;return;}mob.target={feet.x+.5f,static_cast<float>(feet.y),feet.z+.5f};mob.path=std::move(path);mob.pathIndex=0;mob.state=MobAIState::WANDERING;}

void PassiveMobSystem::followParent(PassiveMob& mob,const World& world){const PassiveMob* parent=nullptr;float best=16.f;for(const auto& candidate:m_mobs)if(candidate.id!=mob.id&&candidate.type==mob.type&&!candidate.isBaby()){const float distance=horizontalDistance(mob.position,candidate.position);if(distance<best){best=distance;parent=&candidate;}}if(!parent){mob.state=MobAIState::IDLE;mob.stateTimer=2.f;mob.path.clear();return;}mob.state=MobAIState::FOLLOWING_PARENT;if(best<=3.f){mob.path.clear();mob.pathIndex=0;return;}const glm::ivec3 target=glm::ivec3(glm::floor(parent->position));mob.target=parent->position;mob.path=Pathfinder::findPath(world,glm::ivec3(glm::floor(mob.position)),target);mob.pathIndex=0;}

bool PassiveMobSystem::chooseGrass(PassiveMob& mob,const World& world){const glm::ivec3 center=glm::ivec3(glm::floor(mob.position));float best=std::numeric_limits<float>::max();glm::ivec3 selected{0};bool found=false;for(int dz=-3;dz<=3;++dz)for(int dx=-3;dx<=3;++dx)for(int dy=-1;dy<=1;++dy){glm::ivec3 ground{center.x+dx,center.y-1+dy,center.z+dz};if(world.getBlock(ground.x,ground.y,ground.z)!=BlockType::GRASS)continue;const glm::ivec3 feet=ground+glm::ivec3(0,1,0);if(!Pathfinder::isWalkable(world,feet))continue;const float distance=glm::length(glm::vec2{static_cast<float>(dx),static_cast<float>(dz)});if(distance<best){best=distance;selected=ground;found=true;}}if(!found)return false;mob.target=glm::vec3(selected)+glm::vec3(.5f,1.f,.5f);mob.path=Pathfinder::findPath(world,glm::ivec3(glm::floor(mob.position)),selected+glm::ivec3(0,1,0));mob.pathIndex=0;if(mob.path.empty()&&horizontalDistance(mob.position,mob.target)>=.8f)return false;mob.state=MobAIState::GRAZING;mob.stateTimer=1.5f;return true;}

void PassiveMobSystem::chooseFleePath(PassiveMob& mob,const World& world){glm::vec2 away{mob.position.x-mob.fleeSource.x,mob.position.z-mob.fleeSource.z};if(glm::dot(away,away)<.01f)away={1,0};else away=glm::normalize(away);const int x=std::clamp(static_cast<int>(std::floor(mob.position.x+away.x*8.f)),1,998),z=std::clamp(static_cast<int>(std::floor(mob.position.z+away.y*8.f)),1,998),groundY=surfaceY(world,x,z);if(groundY<0)return;mob.target={x+.5f,groundY+1.f,z+.5f};mob.path=Pathfinder::findPath(world,glm::ivec3(glm::floor(mob.position)),{x,groundY+1,z});mob.pathIndex=0;}

void PassiveMobSystem::updateAI(PassiveMob& mob,World& world){if(mob.state==MobAIState::FLEEING){chooseFleePath(mob,world);return;}if(mob.isBaby()){followParent(mob,world);return;}if(mob.state==MobAIState::GRAZING)return;if(mob.type==MobType::SHEEP&&mob.grazeCooldown<=0&&randomInt(0,4)==0&&chooseGrass(mob,world))return;if(mob.state==MobAIState::IDLE&&mob.stateTimer<=0)chooseWander(mob,world);else if((mob.state==MobAIState::WANDERING||mob.state==MobAIState::FOLLOWING_PARENT)&&mob.pathIndex>=mob.path.size()){mob.state=MobAIState::IDLE;mob.stateTimer=2.f+random01()*3.f;mob.path.clear();}}

void PassiveMobSystem::updateMovement(PassiveMob& mob,float dt,const World& world){glm::vec2 desired{0};if(mob.pathIndex<mob.path.size()){const glm::ivec3 node=mob.path[mob.pathIndex];const glm::vec3 point{node.x+.5f,static_cast<float>(node.y),node.z+.5f};glm::vec2 offset{point.x-mob.position.x,point.z-mob.position.z};if(glm::length(offset)<.25f&&std::abs(point.y-mob.position.y)<.65f){++mob.pathIndex;}else if(glm::dot(offset,offset)>.0001f){desired=glm::normalize(offset);}}
  const float baseSpeed=mob.type==MobType::PIG?1.35f:(mob.type==MobType::SHEEP?1.3f:1.25f),speed=(mob.state==MobAIState::FLEEING?2.2f:baseSpeed)*(mob.isBaby()?1.1f:1.f);mob.velocity.x=desired.x*speed;mob.velocity.z=desired.y*speed;mob.velocity.y=std::max(mob.velocity.y-20.f*dt,-30.f);if(glm::dot(desired,desired)>.01f)mob.yaw=std::atan2(-desired.x,-desired.y);
  constexpr float epsilon=.0001f;
  const bool canStep=mob.onGround&&mob.velocity.y<=0.f&&mobCollides(mob,mob.position-glm::vec3(0,.03f,0),world);
  const float vertical=mob.velocity.y*dt;
  if(std::abs(sweepMob(mob,mob.position,1,vertical,world)-vertical)>epsilon)mob.velocity.y=0.f;
  const glm::vec3 start=mob.position;
  const float dx=mob.velocity.x*dt,dz=mob.velocity.z*dt;
  glm::vec3 normal=start;
  sweepMob(mob,normal,0,dx,world);sweepMob(mob,normal,2,dz,world);
  const bool blocked=std::abs(normal.x-start.x-dx)>epsilon||std::abs(normal.z-start.z-dz)>epsilon;
  mob.position=normal;
  bool steppedUp=false;
  if(canStep&&blocked){
    // Evaluate one combined X/Z step, never an independent rise per axis.
    glm::vec3 stepped=start;
    const float lift=sweepMob(mob,stepped,1,MOB_STEP_HEIGHT,world);
    sweepMob(mob,stepped,0,dx,world);sweepMob(mob,stepped,2,dz,world);
    const float descent=sweepMob(mob,stepped,1,-lift-.001f,world);
    const float rise=stepped.y-start.y;
    const glm::vec2 normalTravel{normal.x-start.x,normal.z-start.z};
    const glm::vec2 stepTravel{stepped.x-start.x,stepped.z-start.z};
    const bool supported=descent>-lift-.001f+epsilon;
    if(supported&&rise>epsilon&&rise<=MOB_STEP_HEIGHT+epsilon&&
       glm::dot(stepTravel,stepTravel)>glm::dot(normalTravel,normalTravel)+epsilon*epsilon&&
       !mobCollides(mob,stepped,world)){
      mob.position=stepped;mob.velocity.y=0.f;
      steppedUp=true;
    }
  }
  if(blocked&&!steppedUp&&mob.pathIndex<mob.path.size()){
    if(++mob.blockedTicks>=10){
      mob.path.clear();mob.pathIndex=0;mob.state=MobAIState::IDLE;mob.stateTimer=2.f+random01()*3.f;
      mob.blockedTicks=0;
    }
  }else if(!blocked||steppedUp)mob.blockedTicks=0;
  if(std::abs(mob.position.x-start.x-dx)>epsilon)mob.velocity.x=0.f;
  if(std::abs(mob.position.z-start.z-dz)>epsilon)mob.velocity.z=0.f;
  const float halfWidth=mobWidth(mob.type)*mobScale(mob)*.5f;mob.position.x=std::clamp(mob.position.x,halfWidth,1000.f-halfWidth);mob.position.z=std::clamp(mob.position.z,halfWidth,1000.f-halfWidth);mob.onGround=mobCollides(mob,mob.position-glm::vec3(0,.03f,0),world);if(glm::length(desired)>.1f)mob.animationTime+=dt*speed*5.f;
}

void PassiveMobSystem::fixedTick(World& world,const glm::vec3& playerPosition){for(auto& mob:m_mobs){if(!active(mob,world,playerPosition))continue;if(mob.age<0)mob.age=std::min(0.f,mob.age+TICK_STEP);mob.hurtTimer=std::max(0.f,mob.hurtTimer-TICK_STEP);mob.grazeCooldown=std::max(0.f,mob.grazeCooldown-TICK_STEP);mob.aiTimer-=TICK_STEP;if(mob.state==MobAIState::FLEEING){mob.stateTimer-=TICK_STEP;if(mob.stateTimer<=0){mob.state=MobAIState::IDLE;mob.stateTimer=2.f+random01()*3.f;mob.path.clear();}}else if(mob.state==MobAIState::IDLE)mob.stateTimer-=TICK_STEP;else if(mob.state==MobAIState::GRAZING&&mob.pathIndex>=mob.path.size()&&horizontalDistance(mob.position,mob.target)<.8f){mob.stateTimer-=TICK_STEP;if(mob.stateTimer<=0){const glm::ivec3 ground=glm::ivec3(glm::floor(mob.target-glm::vec3(0,1.f,0)));if(world.getBlock(ground.x,ground.y,ground.z)==BlockType::GRASS)world.setBlock(ground.x,ground.y,ground.z,BlockType::DIRT);mob.grazeCooldown=20.f+random01()*20.f;mob.state=MobAIState::IDLE;mob.stateTimer=2.f;mob.path.clear();}}if(mob.aiTimer<=0){mob.aiTimer=AI_STEP;updateAI(mob,world);}updateMovement(mob,TICK_STEP,world);}}

bool PassiveMobSystem::spreadGrassAt(World& world,const glm::ivec3& ground){if(world.getBlock(ground.x,ground.y,ground.z)!=BlockType::DIRT||isSolid(world.getBlock(ground.x,ground.y+1,ground.z)))return false;static constexpr int directions[4][2]={{1,0},{-1,0},{0,1},{0,-1}};for(const auto& direction:directions)if(world.getBlock(ground.x+direction[0],ground.y,ground.z+direction[1])==BlockType::GRASS)return world.setBlock(ground.x,ground.y,ground.z,BlockType::GRASS);return false;}
void PassiveMobSystem::spreadGrass(World& world,const glm::vec3& playerPosition){
  const int centerX=static_cast<int>(std::floor(playerPosition.x)),centerZ=static_cast<int>(std::floor(playerPosition.z));
  int spread=0;
  for(int z=centerZ-48;z<=centerZ+48&&spread<8;++z)
    for(int x=centerX-48;x<=centerX+48&&spread<8;++x){
      const int clampedX=std::clamp(x,1,998),clampedZ=std::clamp(z,1,998),y=surfaceY(world,clampedX,clampedZ);
      if(y>=0&&spreadGrassAt(world,{clampedX,y,clampedZ}))++spread;
    }
}

void PassiveMobSystem::update(float dt,World& world,const glm::vec3& playerPosition,float daylight){dt=std::clamp(dt,0.f,.1f);initialize(world,playerPosition,daylight);m_spawnTimer+=dt;m_grassTimer+=dt;if(m_spawnTimer>=SPAWN_INTERVAL){m_spawnTimer=0;if(nearbyCount(playerPosition)<TARGET_NEARBY)spawnPack(static_cast<MobType>(randomInt(0,static_cast<int>(MobType::COUNT)-1)),world,playerPosition,daylight,4);}if(m_grassTimer>=1.f){m_grassTimer=0;spreadGrass(world,playerPosition);}m_tickAccumulator+=dt;while(m_tickAccumulator+1e-6f>=TICK_STEP){m_tickAccumulator=std::max(0.f,m_tickAccumulator-TICK_STEP);fixedTick(world,playerPosition);}}

MobHit PassiveMobSystem::raycast(const glm::vec3& origin,const glm::vec3& direction,float maxDistance)const{MobHit result;if(glm::dot(direction,direction)<=0||maxDistance<=0)return result;const glm::vec3 normalized=glm::normalize(direction);result.distance=maxDistance;for(const auto& mob:m_mobs){const float scale=mobScale(mob),halfWidth=mobWidth(mob.type)*scale*.5f;const glm::vec3 low=mob.position+glm::vec3(-halfWidth,0,-halfWidth),high=mob.position+glm::vec3(halfWidth,mobHeight(mob.type)*scale,halfWidth);float distance=0;if(rayBox(origin,normalized,low,high,result.distance,distance)){result={true,mob.id,distance};}}return result;}

int PassiveMobSystem::attackDamage(const ItemStack& held){return held.kind==ItemKind::TOOL&&!held.empty()?toolAttackDamage(held.toolKind,held.toolTier):1;}

bool PassiveMobSystem::damage(MobId id,int amount,const glm::vec3& source,LootTable& loot,const DropHandler& spawnDrop){auto found=std::find_if(m_mobs.begin(),m_mobs.end(),[&](const PassiveMob& mob){return mob.id==id;});if(found==m_mobs.end()||amount<=0||found->hurtTimer>0)return false;found->health-=amount;found->hurtTimer=.5f;found->fleeSource=source;found->state=MobAIState::FLEEING;found->stateTimer=3.f;found->aiTimer=0;found->path.clear();glm::vec2 away{found->position.x-source.x,found->position.z-source.z};if(glm::dot(away,away)>.01f){away=glm::normalize(away);found->velocity.x=away.x*3.f;found->velocity.z=away.y*3.f;}found->velocity.y=3.f;if(found->health>0)return true;const glm::vec3 position=found->position;const auto drops=loot.mobDrops(found->type,found->isBaby());m_mobs.erase(found);if(spawnDrop)for(const auto& drop:drops)spawnDrop(position-glm::vec3(.5f,.3f,.5f),drop);return true;}

std::vector<MobData> PassiveMobSystem::capture()const{std::vector<MobData> data;data.reserve(m_mobs.size());for(const auto& mob:m_mobs)data.push_back({mob.id,mob.type,mob.position.x,mob.position.y,mob.position.z,mob.yaw,mob.health,mob.age,mob.grazeCooldown});return data;}

bool PassiveMobSystem::restore(const std::vector<MobData>& data){if(data.size()>GLOBAL_CAP)return false;std::unordered_set<MobId> ids;std::vector<PassiveMob> restored;restored.reserve(data.size());MobId next=1;for(const auto& value:data){if(value.id==0||!ids.insert(value.id).second||static_cast<std::size_t>(value.type)>=static_cast<std::size_t>(MobType::COUNT)||!std::isfinite(value.x)||!std::isfinite(value.y)||!std::isfinite(value.z)||!std::isfinite(value.yaw)||!std::isfinite(value.age)||!std::isfinite(value.grazeCooldown)||value.x<0||value.x>=1000||value.z<0||value.z>=1000||value.y<0||value.y>=256||value.health<=0||value.health>mobMaxHealth(value.type)||value.age< BABY_AGE||value.age>0||value.grazeCooldown<0||value.grazeCooldown>60)return false;PassiveMob mob;mob.id=value.id;mob.type=value.type;mob.position={value.x,value.y,value.z};mob.yaw=value.yaw;mob.health=value.health;mob.age=value.age;mob.grazeCooldown=value.grazeCooldown;mob.state=MobAIState::IDLE;mob.stateTimer=2.f;restored.push_back(mob);next=std::max(next,value.id+1);}m_mobs=std::move(restored);m_nextId=next;m_initialized=!m_mobs.empty();m_tickAccumulator=m_spawnTimer=m_grassTimer=0;return true;}
