#include "DayNightCycle.h"
#include <algorithm>
#include <cmath>
#include <glm/geometric.hpp>

namespace {
constexpr float PI = 3.14159265358979323846f;
glm::vec3 mix(const glm::vec3& a,const glm::vec3& b,float t){return a+(b-a)*t;}
float smooth(float t){t=std::clamp(t,0.f,1.f);return t*t*(3.f-2.f*t);}
}

void DayNightCycle::update(float dt){
  m_time=std::fmod(m_time+std::max(dt,0.f)/DAY_LENGTH_SECONDS,1.f);
}

float DayNightCycle::daylight()const{
  float sun=std::clamp(std::sin(m_time*2.f*PI)*1.4f,0.f,1.f);
  return .12f+.88f*smooth(sun);
}

glm::vec3 DayNightCycle::sunDirection()const{
  float angle=m_time*2.f*PI;
  return glm::normalize(glm::vec3{.35f,std::sin(angle),-std::cos(angle)});
}

glm::vec3 DayNightCycle::skyTop()const{
  const glm::vec3 day{.20f,.52f,.86f},sunset{.42f,.20f,.28f},night{.015f,.025f,.08f};
  float light=(daylight()-.12f)/.88f;
  return light<.2f?mix(night,sunset,smooth(light/.2f)):mix(sunset,day,smooth((light-.2f)/.8f));
}

glm::vec3 DayNightCycle::skyBottom()const{
  const glm::vec3 day{.70f,.86f,.96f},sunset{.72f,.36f,.24f},night{.025f,.04f,.10f};
  float light=(daylight()-.12f)/.88f;
  return light<.2f?mix(night,sunset,smooth(light/.2f)):mix(sunset,day,smooth((light-.2f)/.8f));
}
