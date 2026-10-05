#include "world/World.h"
#include "world/WorldGenerator.h"
#include <iostream>

namespace{int fail(const char* message){std::cerr<<message<<'\n';return 1;}}

int main(){
  World world;world.setTaskBudgets(1,1,0);const glm::vec3 center{500.5f,8.f,500.5f};world.update(center);
  if(world.loadedChunkCount()!=1)return fail("streaming exceeded the one-generation frame budget");
  if(!world.isChunkLoadedAt(500,500))return fail("nearest player chunk was not generated first");
  const int surface=WorldGenerator::surfaceHeightAt(500,500,world.seed());
  // Lighting is deferred until the 15-cell halo neighbours exist, so a lone
  // chunk must not light yet.
  if(world.skyLight(500,surface+1,500)==15)return fail("chunk lit before its halo neighbours existed");
  for(int i=0;i<8;++i)world.update(center);
  if(world.loadedChunkCount()!=9)return fail("neighbour chunks did not generate one per update");
  if(world.skyLight(500,surface+1,500)!=15)return fail("chunk did not light once its neighbours existed");
  if(world.isChunkReady(31,31))return fail("chunk became render-ready before its mesh task completed");
  const auto firstPending=world.pendingTaskCount();world.update(center);
  if(world.loadedChunkCount()!=10)return fail("update did not process exactly one more generation");
  if(world.pendingTaskCount()>firstPending+1)return fail("streaming queue accumulated duplicate tasks");

  world.setTaskBudgets(0,0,0);world.update({900.5f,8.f,900.5f});
  if(world.loadedChunkCount()!=0)return fail("distant streamed chunks were not culled");
  const auto distantPending=world.pendingTaskCount();world.update({900.5f,8.f,900.5f});
  if(world.pendingTaskCount()!=distantPending)return fail("repeated updates duplicated pending generation tasks");

  // Mesh upload handoff: with deferred lighting the chunk needs its halo
  // before it lights and meshes.
  World meshWorld;meshWorld.setTaskBudgets(64,64,64);const glm::vec3 mcenter{8.f,8.f,8.f};
  for(int i=0;i<50&&!meshWorld.isChunkReady(0,0);++i)meshWorld.update(mcenter);
  if(!meshWorld.isChunkReady(0,0))return fail("chunk did not become ready once its halo existed");
  auto uploads=meshWorld.takeChunkMeshUploads();
  std::size_t initial=0;bool found=false;
  for(const auto& u:uploads)if(u.position==glm::ivec2(0,0)){initial=u.mesh.indices.size();found=true;}
  if(!found||initial==0)return fail("completed CPU mesh was not handed off for upload");
  if(!meshWorld.takeChunkMeshUploads().empty())return fail("CPU mesh upload hand-off was not drained");
  const int msurface=WorldGenerator::surfaceHeightAt(8,8,meshWorld.seed());
  if(!meshWorld.setBlock(8,msurface+1,8,BlockType::STONE))return fail("mesh edit was rejected");
  for(int i=0;i<50;++i)meshWorld.update(mcenter);
  uploads=meshWorld.takeChunkMeshUploads();
  found=false;std::size_t rebuilt=0;
  for(const auto& u:uploads)if(u.position==glm::ivec2(0,0)){rebuilt=u.mesh.indices.size();found=true;}
  if(!found||rebuilt==initial)return fail("dirty chunk did not hand off its rebuilt CPU mesh");
  return 0;
}
