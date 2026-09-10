#pragma once
#include "PassiveMob.h"
#include "../player/Item.h"
#include <cstdint>
#include <functional>
#include <vector>

class LootTable;
class World;

class PassiveMobSystem{
public:
  using DropHandler=std::function<void(const glm::vec3&,const ItemStack&)>;
  static constexpr std::size_t TARGET_NEARBY=8,LOCAL_CAP=12,GLOBAL_CAP=32;
  static constexpr float TICK_STEP=.05f,AI_STEP=.25f,SPAWN_INTERVAL=20.f,ACTIVE_DISTANCE=80.f,ATTACK_REACH=4.5f;

  explicit PassiveMobSystem(std::uint32_t seed=0x85ebca6bu):m_randomState(seed?seed:1u){}
  const std::vector<PassiveMob>& mobs()const{return m_mobs;}
  PassiveMob& addMob(MobType type,const glm::vec3& position,bool baby=false);
  void clear();
  void initialize(World& world,const glm::vec3& playerPosition,float daylight);
  void update(float dt,World& world,const glm::vec3& playerPosition,float daylight);
  MobHit raycast(const glm::vec3& origin,const glm::vec3& direction,float maxDistance=ATTACK_REACH)const;
  bool damage(MobId id,int amount,const glm::vec3& source,LootTable& loot,const DropHandler& spawnDrop);
  std::vector<MobData> capture()const;
  bool restore(const std::vector<MobData>& data);
  std::size_t nearbyCount(const glm::vec3& position,float radius=ACTIVE_DISTANCE)const;
  bool canSpawnAt(const World& world,const glm::ivec3& feet,float daylight)const;
  static bool spreadGrassAt(World& world,const glm::ivec3& ground);
  static int attackDamage(const ItemStack& held);

private:
  std::vector<PassiveMob> m_mobs;
  MobId m_nextId=1;
  std::uint32_t m_randomState;
  float m_tickAccumulator=0,m_spawnTimer=0,m_grassTimer=0;
  bool m_initialized=false;

  std::uint32_t random();
  float random01();
  int randomInt(int minimum,int maximum);
  bool active(const PassiveMob& mob,const World& world,const glm::vec3& playerPosition)const;
  bool spawnPack(MobType type,World& world,const glm::vec3& playerPosition,float daylight,std::size_t limit);
  bool findSpawnPosition(World& world,const glm::vec3& playerPosition,float daylight,glm::ivec3& feet);
  int surfaceY(const World& world,int x,int z)const;
  void fixedTick(World& world,const glm::vec3& playerPosition);
  void updateAI(PassiveMob& mob,World& world);
  void updateMovement(PassiveMob& mob,float dt,const World& world);
  bool findWanderDestination(const PassiveMob& mob,const World& world,const glm::ivec3& intended,glm::ivec3& target,std::vector<glm::ivec3>& path)const;
  void chooseWander(PassiveMob& mob,const World& world);
  void followParent(PassiveMob& mob,const World& world);
  bool chooseGrass(PassiveMob& mob,const World& world);
  void chooseFleePath(PassiveMob& mob,const World& world);
  void spreadGrass(World& world,const glm::vec3& playerPosition);
};
