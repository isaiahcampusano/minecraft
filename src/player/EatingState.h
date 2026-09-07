#pragma once
#include "Item.h"
#include <algorithm>

struct EatingContext{
  bool rightHeld=false,captured=false,inventoryOpen=false,onGround=false,flying=false,sprinting=false,canEat=false;
  float horizontalSpeed=0;int selectedSlot=0;ItemStack selected{};
};

class EatingState{
public:
  static constexpr float DURATION=1.6f;
  bool active()const{return m_active;}int slot()const{return m_slot;}FoodType food()const{return m_food;}float progress()const{return m_progress;}
  void start(int slot,FoodType food){m_active=true;m_slot=slot;m_food=food;m_progress=0;}
  void cancel(){m_active=false;m_slot=-1;m_progress=0;}
  bool canContinue(const EatingContext& context)const{return m_active&&context.rightHeld&&context.captured&&!context.inventoryOpen&&context.onGround&&!context.flying&&!context.sprinting&&context.horizontalSpeed<=5.01f&&context.canEat&&context.selectedSlot==m_slot&&context.selected.kind==ItemKind::FOOD&&!context.selected.empty()&&context.selected.foodType==m_food;}
  bool update(float dt,bool valid){if(!m_active)return false;if(!valid){cancel();return false;}m_progress+=std::max(0.f,dt);if(m_progress<DURATION)return false;cancel();return true;}
private:
  bool m_active=false;int m_slot=-1;FoodType m_food=FoodType::APPLE;float m_progress=0;
};
