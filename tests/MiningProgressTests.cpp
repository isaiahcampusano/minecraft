#include "player/Player.h"
#include <cmath>
#include <iostream>

namespace {
int fail(const char* message){std::cerr<<message<<'\n';return 1;}
bool near(float a,float b){return std::abs(a-b)<.001f;}
bool advanceFrames(Player& player,BlockType type,float dt,int frames,const ItemStack& held={}){bool broken=false;for(int i=0;i<frames;++i)broken=player.advanceMining(type,dt,held)||broken;return broken;}
}

int main(){
  Player player;
  player.setMiningTarget({1,2,3});
  if(advanceFrames(player,BlockType::STONE,1.f/30.f,60)||!near(player.blockBreakProgress,.5f))return fail("Stone progress did not advance at the hardness rate");
  player.setMiningTarget({1,2,3});
  if(!near(player.blockBreakProgress,.5f))return fail("Stable target reset mining progress");
  player.setMiningTarget({2,2,3});
  if(player.blockBreakProgress!=0.f)return fail("Changing target did not reset progress");
  if(advanceFrames(player,BlockType::GRAVEL,Player::MINING_FIXED_STEP,12)||!near(player.blockBreakProgress,.5f))return fail("Gravel progress rate was incorrect");
  if(!advanceFrames(player,BlockType::GRAVEL,Player::MINING_FIXED_STEP,12)||!near(player.blockBreakProgress,1.f))return fail("Mining did not complete and stop at one");
  player.setMiningTarget({3,2,3});
  if(advanceFrames(player,BlockType::LEAVES,Player::MINING_FIXED_STEP,12)||!near(player.blockBreakProgress,.5f))return fail("Leaves did not use 0.2-second hardness");
  if(!advanceFrames(player,BlockType::LEAVES,Player::MINING_FIXED_STEP,12)||!near(player.blockBreakProgress,1.f))return fail("Leaves did not finish breaking in 0.2 seconds");
  player.resetMiningProgress();
  if(player.blockBreakProgress!=0.f)return fail("Released mining progress did not reset");
  if(player.advanceMining(BlockType::BEDROCK,100.f)||player.blockBreakProgress!=0.f)return fail("Bedrock mining advanced");
  Player player30fps,player120fps;
  player30fps.setMiningTarget({1,2,3});player120fps.setMiningTarget({1,2,3});
  advanceFrames(player30fps,BlockType::STONE,1.f/30.f,60);advanceFrames(player120fps,BlockType::STONE,1.f/120.f,240);
  if(!near(player30fps.blockBreakProgress,.5f)||!near(player120fps.blockBreakProgress,player30fps.blockBreakProgress))return fail("Mining speed changed between 30 FPS and 120 FPS");
  Player lagSpikePlayer;lagSpikePlayer.setMiningTarget({1,2,3});
  if(lagSpikePlayer.advanceMining(BlockType::LEAVES,.5f)||!near(lagSpikePlayer.blockBreakProgress,.25f))return fail("A lag spike bypassed the maximum mining frame time");
  Player cooldownPlayer;
  cooldownPlayer.setMiningTarget({1,2,3});
  cooldownPlayer.startMiningCooldown();
  if(advanceFrames(cooldownPlayer,BlockType::GRAVEL,Player::MINING_FIXED_STEP,12)||cooldownPlayer.blockBreakProgress!=0.f)return fail("Mining advanced during the full-auto cooldown");
  if(advanceFrames(cooldownPlayer,BlockType::GRAVEL,Player::MINING_FIXED_STEP,3)||!near(cooldownPlayer.blockBreakProgress,.025f))return fail("Cooldown boundary did not apply only leftover fixed-step time");
  Player singleFramePlayer,splitFramePlayer;
  singleFramePlayer.setMiningTarget({1,2,3});singleFramePlayer.startMiningCooldown();advanceFrames(singleFramePlayer,BlockType::GRAVEL,1.f/30.f,6);
  splitFramePlayer.setMiningTarget({1,2,3});splitFramePlayer.startMiningCooldown();advanceFrames(splitFramePlayer,BlockType::GRAVEL,1.f/120.f,24);
  if(!near(singleFramePlayer.blockBreakProgress,.4f)||!near(splitFramePlayer.blockBreakProgress,singleFramePlayer.blockBreakProgress))return fail("Cooldown changed when elapsed time was split across frames");
  cooldownPlayer.startMiningCooldown();cooldownPlayer.resetMiningCooldown();cooldownPlayer.resetMiningProgress();
  if(advanceFrames(cooldownPlayer,BlockType::GRAVEL,Player::MINING_FIXED_STEP,12)||!near(cooldownPlayer.blockBreakProgress,.5f))return fail("Reset cooldown delayed a fresh mining interaction");
  Player tappingPlayer;tappingPlayer.setMiningTarget({1,2,3});
  for(int i=0;i<24;++i){if(tappingPlayer.advanceMining(BlockType::GRAVEL,Player::MINING_FIXED_STEP*.5f))return fail("Rapid tapping broke a block faster than holding");tappingPlayer.resetMiningProgress();}
  return 0;
}
