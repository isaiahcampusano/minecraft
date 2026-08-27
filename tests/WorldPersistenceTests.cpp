#include "world/World.h"
#include <iostream>

namespace { int fail(const char* message){std::cerr<<message<<'\n';return 1;} }

int main(){
  World world;
  world.loadChunk(1,1);
  const BlockType untouched=world.getBlock(18,4,18);
  if(!world.setBlock(17,4,17,BlockType::GLASS))return fail("could not place edited block");
  if(!world.setBlock(17,3,17,BlockType::COBBLESTONE))return fail("could not place second edited block");
  if(!world.setBlock(17,2,17,BlockType::AIR))return fail("could not record broken block");
  if(!world.unloadChunk(1,1))return fail("test chunk did not unload");
  world.loadChunk(1,1);
  if(world.getBlock(17,4,17)!=BlockType::GLASS)return fail("placed block was lost after reload");
  if(world.getBlock(17,3,17)!=BlockType::COBBLESTONE)return fail("second edit was lost after reload");
  if(world.getBlock(17,2,17)!=BlockType::AIR)return fail("broken block was regenerated after reload");
  if(world.getBlock(18,4,18)!=untouched)return fail("unedited terrain changed after reload");
  world.unloadChunk(1,1);world.loadChunk(1,1);
  if(world.getBlock(17,4,17)!=BlockType::GLASS)return fail("edit did not survive a second reload");
  return 0;
}
