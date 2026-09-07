#include "SurvivalState.h"
#include <algorithm>
#include <cmath>

void SurvivalState::clampSaturation(){m_saturation=std::clamp(m_saturation,0.f,std::min(MAX_SATURATION,static_cast<float>(m_hunger)));}
void SurvivalState::reset(){m_health=MAX_HEALTH;m_hunger=MAX_HUNGER;m_saturation=5.f;m_exhaustion=m_healTimer=m_starveTimer=m_fallDistance=0;}
void SurvivalState::restore(int health,int hunger,float saturation,float exhaustion){m_health=std::clamp(health,0,MAX_HEALTH);m_hunger=std::clamp(hunger,0,MAX_HUNGER);m_saturation=saturation;m_exhaustion=std::max(0.f,exhaustion);m_healTimer=m_starveTimer=m_fallDistance=0;clampSaturation();}
void SurvivalState::addExhaustion(float amount){m_exhaustion+=std::max(0.f,amount);while(m_exhaustion>=EXHAUSTION_THRESHOLD){m_exhaustion-=EXHAUSTION_THRESHOLD;if(m_saturation>0.f)m_saturation=std::max(0.f,m_saturation-1.f);else if(m_hunger>0)--m_hunger;clampSaturation();}}
void SurvivalState::addSprintDisplacement(float dx,float dz){addExhaustion(.1f*std::sqrt(dx*dx+dz*dz));}
bool SurvivalState::eat(FoodType type){if(!canEat())return false;const auto food=foodProperties(type);if(food.nutrition<=0)return false;m_hunger=std::min(MAX_HUNGER,m_hunger+food.nutrition);m_saturation+=food.saturation;clampSaturation();return true;}
bool SurvivalState::damage(int amount){if(amount<=0||dead())return false;m_health=std::max(0,m_health-amount);m_healTimer=0;return true;}
void SurvivalState::update(float dt){dt=std::max(0.f,dt);if(m_health<MAX_HEALTH&&m_hunger>=18){m_healTimer+=dt;if(m_healTimer>=EFFECT_INTERVAL){++m_health;addExhaustion(6.f);m_healTimer=0;}}else m_healTimer=0;if(m_hunger==0){m_starveTimer+=dt;if(m_starveTimer>=EFFECT_INTERVAL){damage(1);m_starveTimer=0;}}else m_starveTimer=0;}
void SurvivalState::trackFall(float verticalVelocity,float dt){if(verticalVelocity<0)m_fallDistance+=-verticalVelocity*std::max(0.f,dt);}
int SurvivalState::land(){const int amount=m_fallDistance>3.f?static_cast<int>(std::floor(m_fallDistance-3.f)):0;m_fallDistance=0;if(amount>0)damage(amount);return amount;}
