#pragma once
#include "FoodRegistry.h"

class SurvivalState{
public:
  static constexpr int MAX_HEALTH=20,MAX_HUNGER=20;static constexpr float MAX_SATURATION=20.f,EXHAUSTION_THRESHOLD=4.f,EFFECT_INTERVAL=4.f;
  int health()const{return m_health;}int hunger()const{return m_hunger;}float saturation()const{return m_saturation;}float exhaustion()const{return m_exhaustion;}float fallDistance()const{return m_fallDistance;}
  bool dead()const{return m_health<=0;}bool canSprint()const{return m_hunger>6&&!dead();}bool canEat()const{return m_hunger<MAX_HUNGER&&!dead();}
  void reset();void restore(int health,int hunger,float saturation,float exhaustion);
  void addExhaustion(float amount);void addSprintDisplacement(float dx,float dz);bool eat(FoodType type);bool damage(int amount);void update(float dt);
  void trackFall(float verticalVelocity,float dt);int land();void resetFallDistance(){m_fallDistance=0;}
private:
  int m_health=MAX_HEALTH,m_hunger=MAX_HUNGER;float m_saturation=5.f,m_exhaustion=0,m_healTimer=0,m_starveTimer=0,m_fallDistance=0;
  void clampSaturation();
};
