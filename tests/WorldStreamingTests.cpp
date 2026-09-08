#include "world/World.h"
#include <iostream>

namespace{int fail(const char* message){std::cerr<<message<<'\n';return 1;}}

int main(){
  World world;world.setTaskBudgets(1,1,0);const glm::vec3 center{500.5f,8.f,500.5f};world.update(center);
  if(world.loadedChunkCount()!=1)return fail("streaming exceeded the one-generation frame budget");
  if(!world.isChunkLoadedAt(500,500))return fail("nearest player chunk was not generated first");
  if(world.skyLight(500,8,500)!=15)return fail("generated chunk did not receive its budgeted lighting pass");
  if(world.isChunkReady(31,31))return fail("chunk became render-ready before its mesh task completed");
  const auto firstPending=world.pendingTaskCount();world.update(center);
  if(world.loadedChunkCount()!=2)return fail("second update did not process exactly one more generation");
  if(world.pendingTaskCount()>firstPending+1)return fail("streaming queue accumulated duplicate tasks");

  world.setTaskBudgets(0,0,0);world.update({900.5f,8.f,900.5f});
  if(world.loadedChunkCount()!=0)return fail("distant streamed chunks were not culled");
  const auto distantPending=world.pendingTaskCount();world.update({900.5f,8.f,900.5f});
  if(world.pendingTaskCount()!=distantPending)return fail("repeated updates duplicated pending generation tasks");
  return 0;
}
