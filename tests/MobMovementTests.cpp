#include "world/PassiveMobSystem.h"
#include "world/Pathfinder.h"
#include "world/World.h"
#include "world/LootTable.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void reset(World& w){for(int z=506;z<=518;++z)for(int x=506;x<=518;++x){
  w.setBlock(x,6,z,BlockType::GRASS);
  for(int y=7;y<=16;++y)w.setBlock(x,y,z,BlockType::AIR);
}}
bool collides(const PassiveMob& m,const World& w){
  const float half=mobWidth(m.type)*mobScale(m)*.5f,h=mobHeight(m.type)*mobScale(m);
  for(int y=static_cast<int>(std::floor(m.position.y+.0001f));y<=static_cast<int>(std::floor(m.position.y+h-.0001f));++y)
    for(int z=static_cast<int>(std::floor(m.position.z-half+.0001f));z<=static_cast<int>(std::floor(m.position.z+half-.0001f));++z)
      for(int x=static_cast<int>(std::floor(m.position.x-half+.0001f));x<=static_cast<int>(std::floor(m.position.x+half-.0001f));++x)
        if(isSolid(w.getBlock(x,y,z)))return true;
  return false;
}
PassiveMob& setup(PassiveMobSystem& s,MobType type,bool baby,const glm::vec3& start,const std::vector<glm::ivec3>& path){
  auto& m=s.addMob(type,start,baby);m.path=path;m.onGround=true;m.aiTimer=1000;m.stateTimer=1000;m.grazeCooldown=1000;
  m.state=MobAIState::WANDERING;return m;
}
void tick(PassiveMobSystem& s,World& w,int count,bool noJump=true){
  for(int i=0;i<count;++i){float y=s.mobs()[0].position.y;
    s.update(.05f,w,{511.5f,8.f,511.5f},0.f);
    const auto& m=s.mobs()[0];check(!collides(m,w),"mob clipped into terrain");
    check(m.position.y-y<=MOB_STEP_HEIGHT+.0002f,"combined movement climbed more than one block");
    if(noJump)check(m.velocity.y<=0.f,"step applied a jump impulse");
  }
}
}
int main(){try{
  World world;reset(world);
  constexpr int dirs[4][2]={{1,0},{-1,0},{0,1},{0,-1}};
  for(auto type:{MobType::COW,MobType::PIG,MobType::SHEEP})for(bool baby:{false,true})for(const auto& d:dirs){
    reset(world);world.setBlock(511+d[0],7,511+d[1],BlockType::STONE);
    PassiveMobSystem s(1);setup(s,type,baby,{511.5f,7.f,511.5f},{{511+d[0],8,511+d[1]}});
    tick(s,world,60);const auto& m=s.mobs()[0];
    check(m.pathIndex==1&&std::abs(m.position.y-8.f)<.001f&&m.onGround,"species/age/direction failed one-block step");
  }
  reset(world);
  for(int x=511;x<=514;++x)for(int z=510;z<=512;++z)for(int y=7;y<=7+x-511;++y)world.setBlock(x,y,z,BlockType::STONE);
  for(auto type:{MobType::COW,MobType::PIG,MobType::SHEEP})for(bool baby:{false,true}){
    const auto path=Pathfinder::findPath(world,{510,7,511},{514,11,511});check(!path.empty(),"stair path missing");
    PassiveMobSystem s(2);setup(s,type,baby,{510.5f,7.f,511.5f},path);tick(s,world,120);
    check(s.mobs()[0].pathIndex==path.size()&&std::abs(s.mobs()[0].position.y-11.f)<.001f,"stair route failed across chunk seam");
  }
  reset(world);world.setBlock(512,7,512,BlockType::STONE);
  {PassiveMobSystem s(3);setup(s,MobType::COW,false,{511.5f,7.f,511.5f},{{512,8,512}});tick(s,world,80);
   check(s.mobs()[0].pathIndex==1,"diagonal step failed");}
  reset(world);world.setBlock(512,7,511,BlockType::STONE);
  world.setBlock(512,7,512,BlockType::STONE);world.setBlock(512,8,512,BlockType::STONE);
  {PassiveMobSystem s(12);setup(s,MobType::COW,false,{511.54f,7.f,511.54f},{{512,9,512}});tick(s,world,1);
   check(s.mobs()[0].position.y<=8.0002f,"diagonal axes combined two steps");}
  reset(world);
  for(int z=509;z<=513;++z)for(int y=7;y<=8;++y)world.setBlock(512,y,z,BlockType::STONE);
  {PassiveMobSystem s(4);setup(s,MobType::COW,false,{511.5f,7.f,511.5f},{{513,9,511}});tick(s,world,60);
   check(s.mobs()[0].position.x<512.f&&std::abs(s.mobs()[0].position.y-7.f)<.001f,"climbed two-block wall");}
  for(bool baby:{false,true}){
    reset(world);world.setBlock(512,7,511,BlockType::STONE);world.setBlock(511,baby?8:9,511,BlockType::STONE);
    PassiveMobSystem s(5);setup(s,MobType::COW,baby,{511.5f,7.f,511.5f},{{512,8,511}});tick(s,world,60);
    check(s.mobs()[0].position.x<512.f&&std::abs(s.mobs()[0].position.y-7.f)<.001f,"stepped through low ceiling");
  }
  // A suspended beam blocks the body, but provides no reachable landing surface.
  reset(world);world.setBlock(512,8,511,BlockType::STONE);
  {PassiveMobSystem s(6);setup(s,MobType::COW,false,{511.5f,7.f,511.5f},{{513,8,511}});tick(s,world,60);
   check(std::abs(s.mobs()[0].position.y-7.f)<.001f,"accepted unsupported step");}
  // An airborne mob must not snap up a nearby ledge.
  reset(world);world.setBlock(512,7,511,BlockType::STONE);
  {PassiveMobSystem s(7);auto& m=setup(s,MobType::COW,false,{511.54f,7.2f,511.5f},{{512,8,511}});m.onGround=false;
   tick(s,world,1);check(s.mobs()[0].position.y<7.2f,"airborne mob stepped upward");}
  reset(world);
  {PassiveMobSystem s(8);setup(s,MobType::COW,false,{511.5f,7.f,511.5f},{{514,7,511}});tick(s,world,10);
   check(std::abs(s.mobs()[0].position.x-512.125f)<.001f&&std::abs(s.mobs()[0].position.y-7.f)<.001f,"flat walking speed changed");}
  {PassiveMobSystem s(9);auto& m=setup(s,MobType::PIG,true,{511.5f,11.f,511.5f},{});m.onGround=false;m.velocity.y=-30.f;
   tick(s,world,10);check(std::abs(s.mobs()[0].position.y-7.f)<.001f&&s.mobs()[0].onGround,"fall tunneled through floor");}
  reset(world);world.setBlock(511,7,511,BlockType::STONE);
  {PassiveMobSystem s(10);setup(s,MobType::COW,false,{511.5f,8.f,511.5f},{{513,7,511}});tick(s,world,60);
   check(s.mobs()[0].pathIndex==1&&std::abs(s.mobs()[0].position.y-7.f)<.001f,"step down failed");}
  reset(world);
  {PassiveMobSystem s(11);auto& m=setup(s,MobType::COW,false,{511.5f,7.f,511.5f},{});LootTable loot(1);
   check(s.damage(m.id,1,{510.f,7.f,511.5f},loot,{}),"damage failed");m.aiTimer=1000;
   tick(s,world,1,false);check(s.mobs()[0].position.y>7.f&&s.mobs()[0].velocity.y>0.f,"damage knockback lost");}
  std::cout<<"Mob movement regressions passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
