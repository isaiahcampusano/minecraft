#include "world/World.h"
#include <chrono>
#include <iostream>
#include <stdexcept>

namespace {
void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
void prepare(World& world,bool reverse){
  world.setTaskBudgets(0,0,0);
  for(int i=0;i<9;++i){int n=reverse?8-i:i;world.loadChunk(n%3,n/3);}
  for(int z=0;z<48;++z)for(int x=0;x<48;++x)world.setBlock(x,20,z,BlockType::STONE);
  world.setBlock(17,20,17,BlockType::AIR);
  world.updateLighting();
}
}
int main(){try{
  World world;prepare(world,false);
  check(world.skyLight(15,19,15)==11,"diagonal light did not cross chunk corner");
  world.setBlock(17,20,17,BlockType::LEAVES);world.updateLighting();
  check(world.skyLight(15,19,15)==10,"interior neighbor leaf edit did not invalidate diagonal chunk");
  world.setBlock(17,20,17,BlockType::STONE);world.updateLighting();
  check(world.skyLight(15,19,15)==0,"roof closure left stale light across seam");
  world.setBlock(17,20,17,BlockType::AIR);world.updateLighting();
  check(world.skyLight(15,19,15)==11,"roof opening failed to restore light");
  world.unloadChunk(1,1);world.updateLighting();
  check(world.skyLight(15,19,15)==0,"unloaded chunk leaked light");
  world.loadChunk(1,1);world.updateLighting();
  check(world.skyLight(15,19,15)==11,"chunk reload did not restore diagonal illumination");
  auto pending=world.pendingTaskCount();
  world.setBlock(17,20,17,BlockType::AIR);world.setBlock(17,20,17,BlockType::AIR);
  check(world.pendingTaskCount()==pending,"duplicate lighting tasks");
  World reverse;prepare(reverse,true);
  for(int z=1;z<32;++z)for(int x=1;x<32;++x)
    check(world.skyLight(x,19,z)==reverse.skyLight(x,19,z),"lighting depends on chunk insertion / recompute order");
  World boundary;boundary.loadChunk(0,0);boundary.updateLighting();
  check(boundary.skyLight(0,100,0)==15,"finite world boundary lost sky");
  World stream;stream.setTaskBudgets(1,1,0);
  const auto start=std::chrono::steady_clock::now();
  for(int i=0;i<20;++i)stream.update({500.f,10.f,500.f});
  check(stream.loadedChunkCount()==20,"generation task budget changed");
  std::cout<<"Streaming update mean (one generation + one lighting task): "
           <<std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/20<<" ms\n";
  return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
