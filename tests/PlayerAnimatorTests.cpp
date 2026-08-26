#include "player/Player.h"
#include "player/PlayerAnimator.h"
#include <cmath>
#include <iostream>

namespace{int fail(const char*m){std::cerr<<m<<'\n';return 1;}void tick(PlayerAnimator&a,const Player&p,float seconds){for(int i=0;i<static_cast<int>(seconds*60);++i)a.update(p,1.f/60.f);}}
int main(){
  Player player;PlayerAnimator animator;player.onGround=true;player.velocity={5.f,0,0};tick(animator,player,.25f);auto walk=animator.pose();
  if(walk.leftArm*walk.rightArm>=0||walk.leftLeg*walk.rightLeg>=0)return fail("Walking limbs are not opposing");
  if(walk.leftArm*walk.leftLeg>=0)return fail("Same-side arm and leg should oppose each other");

  player.velocity={0,8.f,0};player.onGround=false;tick(animator,player,.5f);auto jump=animator.pose();
  if(jump.leftArm>-.9f||jump.rightArm>-.9f)return fail("Jump pose did not raise both arms");

  player.velocity={0,-8.f,0};tick(animator,player,.5f);auto fall=animator.pose();
  if(fall.leftArm<.25f||fall.rightArm>-.25f)return fail("Falling pose was not applied");

  player.isFlying=true;tick(animator,player,.5f);if(animator.pose().torsoLean<.5f)return fail("Flying pose did not lean the torso");
  player.isFlying=false;player.onGround=true;player.velocity=glm::vec3(0);tick(animator,player,1.f);auto idle=animator.pose();
  if(std::abs(idle.leftArm)>.02f||std::abs(idle.rightLeg)>.02f)return fail("Limbs did not smoothly return to idle");
  return 0;
}
