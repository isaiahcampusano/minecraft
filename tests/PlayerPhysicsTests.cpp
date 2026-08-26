#include "player/Player.h"
#include "world/World.h"
#include <cmath>
#include <iostream>

namespace {
bool near(float a,float b,float tolerance=.02f){return std::abs(a-b)<=tolerance;}
int fail(const char* message){std::cerr<<message<<'\n';return 1;}
}

int main(){
  World world;
  world.setBlock(500,4,500,BlockType::GRASS);
  Player player({500.5f,8.f,500.5f});
  for(int i=0;i<300;++i)player.update(1.f/60.f,world);
  if(!player.onGround||!near(player.position.y,5.f))return fail("Player did not land on the grass surface");

  player.jump();
  player.update(1.f/60.f,world);
  if(player.position.y<=5.f||player.velocity.y<=0)return fail("Grounded player did not jump");

  for(int y=1;y<=4;++y)world.setBlock(500,y,500,BlockType::AIR);
  player.position={500.5f,5.f,500.5f};player.velocity=glm::vec3(0);player.onGround=false;
  for(int i=0;i<300;++i)player.update(1.f/60.f,world);
  if(!player.onGround||!near(player.position.y,1.f))return fail("Player did not fall through the dug hole onto bedrock");

  player.toggleFly();player.velocity.y=5.f;float before=player.position.y;player.update(.1f,world);
  if(!near(player.position.y,before+.5f)||!near(player.velocity.y,5.f))return fail("Flying movement was affected by gravity");
  return 0;
}
