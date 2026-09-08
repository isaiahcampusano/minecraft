#include "world/Pathfinder.h"
#include "world/World.h"
#include <iostream>

namespace{
int fail(const char* message){std::cerr<<message<<'\n';return 1;}
void flatten(World& world,int centerX,int centerZ,int radius){for(int z=centerZ-radius;z<=centerZ+radius;++z)for(int x=centerX-radius;x<=centerX+radius;++x){world.setBlock(x,6,z,BlockType::GRASS);world.setBlock(x,7,z,BlockType::AIR);world.setBlock(x,8,z,BlockType::AIR);world.setBlock(x,9,z,BlockType::AIR);}}
}

int main(){World world;for(int z=30;z<=32;++z)for(int x=30;x<=34;++x)world.loadChunk(x,z);flatten(world,500,500,10);const glm::ivec3 start{500,7,500},target{505,7,500};const auto flat=Pathfinder::findPath(world,start,target);if(flat.empty()||flat.back()!=target)return fail("flat-ground path did not reach its target");world.setBlock(502,7,500,BlockType::STONE);world.setBlock(502,8,500,BlockType::STONE);const auto around=Pathfinder::findPath(world,start,target);if(around.empty())return fail("pathfinder did not route around a two-block obstacle");for(const auto& node:around)if(node.x==502&&node.z==500)return fail("path crossed a blocked cell");flatten(world,500,520,3);world.setBlock(501,7,520,BlockType::GRASS);const auto step=Pathfinder::findPath(world,{500,7,520},{501,8,520});if(step.empty()||step.back().y!=8)return fail("pathfinder could not climb one block");flatten(world,540,500,4);const glm::ivec3 enclosed{542,7,500};for(const glm::ivec3& offset:{glm::ivec3{1,0,0},glm::ivec3{-1,0,0},glm::ivec3{0,0,1},glm::ivec3{0,0,-1}}){world.setBlock(enclosed.x+offset.x,7,enclosed.z+offset.z,BlockType::STONE);world.setBlock(enclosed.x+offset.x,8,enclosed.z+offset.z,BlockType::STONE);}if(!Pathfinder::findPath(world,{540,7,500},enclosed).empty())return fail("unreachable target produced a path");if(!Pathfinder::findPath(world,start,target,16,1).empty())return fail("search limit was ignored");return 0;}
