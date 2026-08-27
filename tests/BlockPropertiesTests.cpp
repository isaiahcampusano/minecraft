#include "world/BlockProperties.h"
#include <cmath>
#include <iostream>

namespace { int fail(const char* message){std::cerr<<message<<'\n';return 1;} bool near(float a,float b){return std::abs(a-b)<.001f;} }

int main(){
  const struct { BlockType type; float hardness; } expected[]={
    {BlockType::GRASS,1.2f},{BlockType::DIRT,.5f},{BlockType::STONE,4.f},
    {BlockType::COBBLESTONE,10.f},{BlockType::OAK_LOG,2.f},{BlockType::SAND,.6f},
    {BlockType::GRAVEL,.2f},{BlockType::GLASS,.6f}
  };
  if(getBlockProperties(BlockType::AIR).diggable)return fail("Air was diggable");
  const auto& bedrock=getBlockProperties(BlockType::BEDROCK);
  if(bedrock.diggable||!near(bedrock.hardness,-1.f))return fail("Bedrock was not unbreakable");
  for(const auto& item:expected){const auto& properties=getBlockProperties(item.type);if(!properties.diggable||properties.hardness<=0.f||!near(properties.hardness,item.hardness))return fail("Block hardness registry did not match the specification");}
  return 0;
}
