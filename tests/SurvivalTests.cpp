#include "player/SurvivalState.h"
#include "player/SurvivalHud.h"
#include <cmath>
#include <iostream>

namespace{int fail(const char* message){std::cerr<<message<<'\n';return 1;}bool near(float a,float b){return std::fabs(a-b)<0.0001f;}}

int main(){
  SurvivalState state;state.restore(20,20,2.f,3.5f);state.addExhaustion(0.75f);
  if(!near(state.exhaustion(),0.25f)||!near(state.saturation(),1.f)||state.hunger()!=20)return fail("exhaustion remainder or saturation-first consumption failed");
  state.addExhaustion(8.f);if(state.saturation()!=0.f||state.hunger()!=19||!near(state.exhaustion(),0.25f))return fail("repeated exhaustion thresholds failed");
  state.restore(20,5,5.f,0);state.addExhaustion(4.f);if(state.saturation()!=4.f||state.hunger()!=5)return fail("saturation was not consumed before hunger");
  state.restore(20,5,.5f,0);state.addExhaustion(4.f);if(state.saturation()!=0.f||state.hunger()!=5)return fail("fractional saturation was not depleted before hunger");
  state.restore(20,20,5.f,0);state.addSprintDisplacement(3.f,4.f);if(!near(state.exhaustion(),.5f))return fail("horizontal sprint displacement cost failed");
  state.restore(20,1,0,0);if(!state.eat(FoodType::APPLE)||state.hunger()!=5||!near(state.saturation(),2.4f))return fail("apple nutrition failed");
  state.restore(20,19,19.f,0);if(!state.eat(FoodType::APPLE)||state.hunger()!=20||state.saturation()!=20.f)return fail("apple saturation cap failed");
  state.restore(20,20,5,0);if(state.eat(FoodType::APPLE))return fail("full-hunger eating was accepted");
  state.restore(19,18,0,0);state.update(3.99f);if(state.health()!=19)return fail("regeneration fired early");state.update(.01f);if(state.health()!=20||!near(state.exhaustion(),2.f)||state.hunger()!=17)return fail("regeneration or exhaustion failed");
  state.restore(18,20,5,0);state.update(8.f);if(state.health()!=19)return fail("regeneration carried timer overflow");
  state.restore(19,17,0,0);state.update(3.f);state.restore(19,18,0,0);state.update(1.1f);if(state.health()!=19)return fail("regeneration timer did not reset when ineligible");
  state.restore(2,0,0,0);state.update(4.f);if(state.health()!=1)return fail("starvation interval failed");state.update(4.f);if(!state.dead())return fail("starvation was not lethal");
  state.restore(3,0,0,0);state.update(8.f);if(state.health()!=2)return fail("starvation carried timer overflow");
  state.restore(20,0,0,0);state.update(3.f);state.restore(20,1,0,0);state.update(2.f);state.restore(20,0,0,0);state.update(2.1f);if(state.health()!=20)return fail("starvation timer did not reset");
  state.restore(20,20,5,0);state.trackFall(-5.f,1.f);if(state.land()!=2||state.health()!=18||state.fallDistance()!=0)return fail("fall damage formula/reset failed");state.trackFall(-2.f,1.f);if(state.land()!=0||state.health()!=18)return fail("safe fall dealt damage");
  state.restore(1,20,5,0);if(!state.damage(1)||!state.dead())return fail("lethal damage failed");state.reset();if(state.health()!=20||state.hunger()!=20||state.saturation()!=5.f)return fail("respawn defaults failed");
  if(survivalIconValue(20,0)!=2||survivalIconValue(20,9)!=2||survivalIconValue(19,9)!=1||survivalIconValue(1,0)!=1||survivalIconValue(0,0)!=0)return fail("survival HUD icon states failed");
  return 0;
}
