#include "world/FurnaceState.h"
#include "player/CraftingRecipe.h"
#include "player/InventoryLayout.h"
#include "player/FoodRegistry.h"
#include "player/SurvivalState.h"
#include "world/BlockProperties.h"
#include <iostream>
#include <limits>

#define CHECK(x) do { if(!(x)){std::cerr<<__LINE__<<": "<<#x<<'\n';return 1;} } while(false)
bool near(double a,double b){return std::abs(a-b)<1e-6;}
FurnaceState stocked(){FurnaceState f;f.inputSlot=ItemStack::food(FoodType::RAW_BEEF,16);f.fuelSlot=ItemStack::block(BlockType::PLANKS,16);return f;}
int main(){
  CHECK(SmeltingRegistry::recipes().size()==3);
  for(const auto& r:SmeltingRegistry::recipes()){
    CHECK(SmeltingRegistry::match(r.input)==&r);CHECK(r.cookTimeSeconds==10.);
    FurnaceState f;f.inputSlot=ItemStack::food(r.input);f.fuelSlot=ItemStack::material(MaterialType::STICK,2);f.tick(10.);
    CHECK(f.outputSlot.foodType==r.output&&f.outputSlot.count==1&&f.inputSlot.empty()&&!f.lit());
  }
  CHECK(!SmeltingRegistry::match(FoodType::APPLE));CHECK(!SmeltingRegistry::match(ItemStack::block(BlockType::DIRT)));
  CHECK(!SmeltingRegistry::match(static_cast<FoodType>(255)));
  CHECK(FuelRegistry::burnTime(ItemStack::block(BlockType::PLANKS))==15.);
  CHECK(FuelRegistry::burnTime(ItemStack::block(BlockType::OAK_LOG))==15.);
  CHECK(FuelRegistry::burnTime(ItemStack::material(MaterialType::STICK))==5.);
  CHECK(FuelRegistry::burnTime(ItemStack::food(FoodType::RAW_BEEF))==0.);
  CHECK(FuelRegistry::burnTime(ItemStack{})==0.);
  auto f=stocked();f.fuelSlot=ItemStack::material(MaterialType::STICK);f.tick(5.);
  CHECK(near(f.cookProgress,5.)&&f.outputSlot.empty()&&f.fuelSlot.empty()&&!f.lit());
  f.tick(100.);CHECK(near(f.cookProgress,5.));
  ItemStack cursor=ItemStack::material(MaterialType::STICK);CHECK(f.interact(FurnaceSlot::Fuel,cursor));f.tick(5.);
  CHECK(f.outputSlot.count==1&&near(f.cookProgress,0.));
  f=stocked();f.fuelSlot=ItemStack::block(BlockType::PLANKS);f.tick(15.);
  CHECK(f.outputSlot.count==1&&near(f.cookProgress,5.)&&!f.lit()&&f.fuelSlot.empty());
  f=stocked();f.outputSlot=ItemStack::food(FoodType::COOKED_BEEF,64);f.tick(20.);
  CHECK(f.fuelSlot.count==16&&!f.lit()&&f.cookProgress==0.);
  f=stocked();f.tick(2.);f.outputSlot=ItemStack::food(FoodType::COOKED_BEEF,64);f.tick(30.);
  CHECK(!f.lit()&&f.fuelSlot.count==15&&near(f.cookProgress,2.));
  cursor={};CHECK(f.interact(FurnaceSlot::Output,cursor));f.tick(8.);CHECK(f.outputSlot.count==1);
  f=stocked();f.tick(3.);cursor=ItemStack::food(FoodType::RAW_BEEF,2);CHECK(f.interact(FurnaceSlot::Input,cursor));CHECK(near(f.cookProgress,3.));
  cursor=ItemStack::food(FoodType::RAW_MUTTON,2);CHECK(f.interact(FurnaceSlot::Input,cursor));CHECK(f.cookProgress==0.&&cursor.foodType==FoodType::RAW_BEEF);
  f.tick(2.);cursor={};CHECK(f.interact(FurnaceSlot::Input,cursor));CHECK(f.cookProgress==0.);
  f.tick(100.);CHECK(f.outputSlot.empty()&&f.fuelSlot.count==15&&!f.lit());
  f=stocked();f.fuelSlot={};f.tick(10.);CHECK(f.cookProgress==0.);
  f=stocked();f.inputSlot={};f.tick(10.);CHECK(f.fuelSlot.count==16);
  for(double step:{.01,.03,.1,.7,3.}){
    auto big=stocked(),small=stocked();big.tick(73.);
    double remaining=73.;while(remaining>1e-8){const double dt=std::min(step,remaining);small.tick(dt);remaining-=dt;}
    CHECK(big.outputSlot.count==small.outputSlot.count&&big.inputSlot.count==small.inputSlot.count&&big.fuelSlot.count==small.fuelSlot.count);
    CHECK(near(big.cookProgress,small.cookProgress)&&near(big.fuelRemaining,small.fuelRemaining));
  }
  f=stocked();f.tick(-10.);f.tick(std::numeric_limits<double>::quiet_NaN());CHECK(f.fuelSlot.count==16&&f.cookProgress==0.);
  f=stocked();f.tick(1.);cursor=ItemStack::material(MaterialType::STICK,64);CHECK(f.interact(FurnaceSlot::Fuel,cursor));CHECK(f.fuelDuration==15.&&f.fuelRemaining==14.);
  f=stocked();cursor=ItemStack::food(FoodType::APPLE);CHECK(!f.interact(FurnaceSlot::Input,cursor));CHECK(cursor.foodType==FoodType::APPLE&&f.inputSlot.count==16);
  cursor=ItemStack::food(FoodType::RAW_BEEF,64);CHECK(f.interact(FurnaceSlot::Input,cursor));CHECK(cursor.count==16&&f.inputSlot.count==64);
  CHECK(!f.interact(FurnaceSlot::Input,cursor));
  f.outputSlot=ItemStack::food(FoodType::COOKED_BEEF,9);cursor=ItemStack::food(FoodType::COOKED_BEEF,62);
  CHECK(f.interact(FurnaceSlot::Output,cursor));CHECK(cursor.count==64&&f.outputSlot.count==7);
  cursor=ItemStack::food(FoodType::COOKED_MUTTON);CHECK(!f.interact(FurnaceSlot::Output,cursor));CHECK(f.outputSlot.count==7);
  cursor={};CHECK(f.interact(FurnaceSlot::Output,cursor));CHECK(cursor.count==7&&f.outputSlot.empty());CHECK(!f.interact(FurnaceSlot::Output,cursor));
  std::array<ItemStack,9> grid;grid.fill(ItemStack::block(BlockType::COBBLESTONE));grid[4]={};
  const auto* recipe=CraftingRegistry::match(grid,3,3);CHECK(recipe&&recipe->output.blockType==BlockType::FURNACE);
  CHECK(!CraftingRegistry::match(grid,2,2));grid[4]=ItemStack::block(BlockType::COBBLESTONE);CHECK(!CraftingRegistry::match(grid,3,3));
  grid[4]={};grid[0]={};CHECK(!CraftingRegistry::match(grid,3,3));
  CHECK(getBlockProperties(BlockType::FURNACE).hardness==3.5f);
  CHECK(!canDropBlock(BlockType::FURNACE,ItemStack{}));CHECK(canDropBlock(BlockType::FURNACE,ItemStack::tool(ToolKind::PICKAXE,ToolTier::WOOD,50)));
  for(int width:{800,1280,1920})for(int i=0;i<3;++i){const auto r=FurnaceLayout::slots(width)[i];
    const auto hit=InventoryLayout::hitTest(r.x+24,1080-(r.y+24),width,1080,false,false,true);
    CHECK(hit.area==InventoryLayout::Area::FURNACE&&hit.index==i);
    CHECK(FurnaceLayout::hitTest(r.x+r.w,r.y,width)!=i);
  }
  for(const auto& r:SmeltingRegistry::recipes()){
    SurvivalState survival;survival.restore(20,0,0,0);CHECK(survival.eat(r.output));
    const auto props=foodProperties(r.output);CHECK(survival.hunger()==props.nutrition&&near(survival.saturation(),props.saturation));
  }
  return 0;
}
