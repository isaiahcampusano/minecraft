#pragma once
#include "../player/SmeltingRecipe.h"
#include "../player/FuelRegistry.h"
#include <algorithm>
#include <cmath>
#include <utility>

enum class FurnaceSlot { Input, Fuel, Output };

struct FurnaceState {
  ItemStack inputSlot, fuelSlot, outputSlot;
  double fuelRemaining=0., fuelDuration=0., cookProgress=0.;
  bool lit()const{return fuelRemaining>0.;}
  bool isDefault()const{return inputSlot.empty()&&fuelSlot.empty()&&outputSlot.empty()&&fuelRemaining==0.&&fuelDuration==0.&&cookProgress==0.;}
  static bool accepts(FurnaceSlot slot,const ItemStack& item){
    if(slot==FurnaceSlot::Output)return false;
    return item.empty()||(slot==FurnaceSlot::Input?SmeltingRegistry::match(item)!=nullptr:FuelRegistry::burnTime(item)>0.);
  }
  static bool validStack(const ItemStack& item){
    if(item.empty())return item.kind==ItemKind::BLOCK&&item.blockType==BlockType::AIR&&item.count==0&&item.durability==0;
    return item.count>0&&item.count<=64&&item.durability==0;
  }
  bool valid()const{
    if(!validStack(inputSlot)||!validStack(fuelSlot)||!validStack(outputSlot)||!accepts(FurnaceSlot::Input,inputSlot)||!accepts(FurnaceSlot::Fuel,fuelSlot))return false;
    if(!outputSlot.empty()&&(outputSlot.kind!=ItemKind::FOOD||outputSlot.foodType<FoodType::COOKED_BEEF||outputSlot.foodType>FoodType::COOKED_MUTTON))return false;
    return std::isfinite(fuelRemaining)&&std::isfinite(fuelDuration)&&std::isfinite(cookProgress)&&
      fuelRemaining>=0.&&fuelRemaining<=fuelDuration&&(fuelDuration==0.||fuelDuration==5.||fuelDuration==15.)&&
      cookProgress>=0.&&cookProgress<10.&&(cookProgress==0.||SmeltingRegistry::match(inputSlot));
  }
  // This is the only UI mutation path; recipe progress belongs to the input type.
  bool interact(FurnaceSlot slot,ItemStack& cursor){
    ItemStack& item=slot==FurnaceSlot::Input?inputSlot:slot==FurnaceSlot::Fuel?fuelSlot:outputSlot;
    const ItemStack previous=inputSlot;
    if(slot==FurnaceSlot::Output){
      if(item.empty()||(!cursor.empty()&&!sameItemType(item,cursor)))return false;
      const int amount=std::min(item.count,64-(cursor.empty()?0:cursor.count));
      if(amount<=0)return false;
      if(cursor.empty())cursor=ItemStack::food(item.foodType,amount);else cursor.count+=amount;
      item.count-=amount;if(item.count==0)item={};
    }else{
      if(!accepts(slot,cursor))return false;
      if(cursor.empty()&&item.empty())return false;
      if(!cursor.empty()&&!item.empty()&&sameItemType(cursor,item)){
        const int amount=std::min(cursor.count,64-item.count);if(amount<=0)return false;
        item.count+=amount;cursor.count-=amount;if(cursor.count==0)cursor={};
      }else std::swap(item,cursor);
    }
    if(inputSlot.empty()||previous.empty()||!sameItemType(previous,inputSlot))cookProgress=0.;
    return true;
  }
  void tick(double dt){
    if(!(dt>0.)||!std::isfinite(dt))return;
    constexpr double epsilon=1e-9;
    while(dt>epsilon){
      const auto* recipe=SmeltingRegistry::match(inputSlot);
      const bool canCook=recipe&&(outputSlot.empty()||(outputSlot.kind==ItemKind::FOOD&&outputSlot.foodType==recipe->output&&outputSlot.count<64));
      if(!lit()){
        fuelRemaining=0.;fuelDuration=0.;
        if(!canCook)return;
        const double duration=FuelRegistry::burnTime(fuelSlot);if(duration<=0.)return;
        fuelRemaining=fuelDuration=duration;
        if(--fuelSlot.count==0)fuelSlot={};
      }
      double step=std::min(dt,fuelRemaining);
      if(canCook)step=std::min(step,recipe->cookTimeSeconds-cookProgress);
      fuelRemaining-=step;dt-=step;if(canCook)cookProgress+=step;
      if(canCook&&cookProgress+epsilon>=recipe->cookTimeSeconds){
        const FoodType output=recipe->output;
        if(--inputSlot.count==0)inputSlot={};
        if(outputSlot.empty())outputSlot=ItemStack::food(output);else ++outputSlot.count;
        cookProgress=0.;
      }
      if(fuelRemaining<epsilon){fuelRemaining=0.;fuelDuration=0.;}
    }
  }
};
