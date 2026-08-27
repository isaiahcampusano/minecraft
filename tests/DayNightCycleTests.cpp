#include "core/DayNightCycle.h"
#include <cmath>
#include <iostream>
#include <glm/geometric.hpp>

namespace { int fail(const char* message){std::cerr<<message<<'\n';return 1;} bool near(float a,float b,float tolerance=.002f){return std::abs(a-b)<=tolerance;} }

int main(){
  DayNightCycle cycle;
  if(cycle.daylight()<.99f)return fail("Initial noon daylight was not near one");
  cycle.update(DayNightCycle::DAY_LENGTH_SECONDS*.5f);
  if(cycle.daylight()>.13f)return fail("Midnight daylight was not near its floor");

  DayNightCycle wrapping;
  wrapping.update(DayNightCycle::DAY_LENGTH_SECONDS*.75f);
  if(!near(wrapping.time(),0.f))return fail("Cycle did not wrap to zero");
  wrapping.update(DayNightCycle::DAY_LENGTH_SECONDS);
  if(!near(wrapping.time(),0.f))return fail("A full day did not preserve wrapped time");

  DayNightCycle direction;
  for(int i=0;i<8;++i){glm::vec3 sun=direction.sunDirection();if(!near(glm::length(sun),1.f))return fail("Sun direction was not a unit vector");direction.update(DayNightCycle::DAY_LENGTH_SECONDS/8.f);}
  return 0;
}
