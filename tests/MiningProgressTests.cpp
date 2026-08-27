#include "player/Player.h"
#include <cmath>
#include <iostream>

namespace { int fail(const char* message){std::cerr<<message<<'\n';return 1;} bool near(float a,float b){return std::abs(a-b)<.001f;} }

int main(){
  Player player;
  player.setMiningTarget({1,2,3});
  if(player.advanceMining(BlockType::STONE,2.f)||!near(player.blockBreakProgress,.5f))return fail("Stone progress did not advance at the hardness rate");
  player.setMiningTarget({1,2,3});
  if(!near(player.blockBreakProgress,.5f))return fail("Stable target reset mining progress");
  player.setMiningTarget({2,2,3});
  if(player.blockBreakProgress!=0.f)return fail("Changing target did not reset progress");
  if(player.advanceMining(BlockType::GRAVEL,.1f)||!near(player.blockBreakProgress,.5f))return fail("Gravel progress rate was incorrect");
  if(!player.advanceMining(BlockType::GRAVEL,.1f)||!near(player.blockBreakProgress,1.f))return fail("Mining did not complete and stop at one");
  player.resetMiningProgress();
  if(player.blockBreakProgress!=0.f)return fail("Released mining progress did not reset");
  if(player.advanceMining(BlockType::BEDROCK,100.f)||player.blockBreakProgress!=0.f)return fail("Bedrock mining advanced");
  return 0;
}
