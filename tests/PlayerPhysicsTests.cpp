#include "player/Player.h"
#include "world/World.h"
#include "player/Respawn.h"
#include <cmath>
#include <iostream>

namespace {
bool near(float a,float b,float tolerance=.02f){return std::abs(a-b)<=tolerance;}
int fail(const char* message){std::cerr<<message<<'\n';return 1;}
}

int main(){
  World world;
  world.setBlock(500,6,500,BlockType::GRASS);
  Player player({500.5f,8.f,500.5f});
  for(int i=0;i<300;++i)player.update(1.f/60.f,world);
  if(!player.onGround||!near(player.position.y,7.f))return fail("Player did not land on the grass surface");

  if(!player.jump()||!near(player.survival.exhaustion(),.05f))return fail("Grounded jump did not report success or charge exhaustion");
  player.update(1.f/60.f,world);
  if(player.position.y<=7.f||player.velocity.y<=0)return fail("Grounded player did not jump");
  if(player.jump())return fail("Airborne jump incorrectly succeeded");

  for(int y=1;y<=6;++y)world.setBlock(500,y,500,BlockType::AIR);
  player.position={500.5f,7.f,500.5f};player.velocity=glm::vec3(0);player.onGround=false;
  for(int i=0;i<300;++i)player.update(1.f/60.f,world);
  if(!player.onGround||!near(player.position.y,1.f))return fail("Player did not fall through the dug hole onto bedrock");

  player.toggleFly();player.velocity.y=5.f;float before=player.position.y;player.update(.1f,world);
  if(!near(player.position.y,before+.5f)||!near(player.velocity.y,5.f))return fail("Flying movement was affected by gravity");
  player.toggleFly();player.survival.trackFall(-20.f,1.f);player.toggleFly();if(player.survival.fallDistance()!=0)return fail("entering flight did not reset fall distance");player.toggleFly();player.survival.trackFall(-2.f,1.f);if(player.survival.fallDistance()<=0)return fail("falling after leaving flight did not accumulate distance");
  World spawnWorld;spawnWorld.loadChunk(31,31);spawnWorld.setBlock(500,7,500,BlockType::STONE);spawnWorld.setBlock(500,8,500,BlockType::AIR);spawnWorld.setBlock(500,9,500,BlockType::AIR);const auto safe=safeRespawnPosition(spawnWorld);if(!near(safe.x,500.5f)||!near(safe.y,8.f)||!near(safe.z,500.5f))return fail("safe respawn candidate was not selected");for(int y=7;y<=21;++y)spawnWorld.setBlock(500,y,500,BlockType::STONE);const auto fallback=safeRespawnPosition(spawnWorld);if(!near(fallback.x,500.f)||!near(fallback.y,8.f)||!near(fallback.z,500.f))return fail("safe respawn fallback changed");
  return 0;
}
