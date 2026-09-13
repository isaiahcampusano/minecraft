#include "world/World.h"
#include "core/SaveLoad.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include <cstring>

#define CHECK(x) do { if(!(x)){std::cerr<<__LINE__<<": "<<#x<<'\n';return 1;} } while(false)
SaveData::SlotData food(FoodType type,int count){SaveData::SlotData s;s.kind=ItemKind::FOOD;s.foodType=type;s.count=static_cast<std::uint8_t>(count);return s;}
std::vector<char> bytes(const std::filesystem::path& p){std::ifstream in(p,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
void write(const std::filesystem::path& p,const std::vector<char>& data){std::ofstream out(p,std::ios::binary|std::ios::trunc);out.write(data.data(),data.size());}
int main(){
  World world;const glm::ivec3 p{17,8,17},q{33,8,33};
  CHECK(world.setBlock(p.x,p.y,p.z,BlockType::FURNACE));CHECK(world.setBlock(q.x,q.y,q.z,BlockType::FURNACE));
  ItemStack cursor=ItemStack::food(FoodType::RAW_BEEF,3);CHECK(world.interactFurnace(p,FurnaceSlot::Input,cursor));
  cursor=ItemStack::block(BlockType::PLANKS,2);CHECK(world.interactFurnace(p,FurnaceSlot::Fuel,cursor));
  world.tickFurnaces(12.);CHECK(world.furnaceAt(p)->outputSlot.count==1&&world.furnaceAt(p)->cookProgress==2.);
  CHECK(world.furnaceAt(q)->isDefault());world.tickFurnaces(0.);CHECK(world.furnaceAt(p)->fuelRemaining==3.);
  CHECK(world.unloadChunk(1,1));world.tickFurnaces(50.);CHECK(!world.furnaceAt(p));
  const auto state=world.captureFurnaces();CHECK(state.size()==1&&state[0].state.fuelRemaining==3.);
  CHECK(!world.interactFurnace(p,FurnaceSlot::Output,cursor));
  world.loadChunk(1,1);CHECK(world.furnaceAt(p)->cookProgress==2.);
  CHECK(world.setBlock(p.x,p.y,p.z,BlockType::FURNACE));CHECK(world.furnaceAt(p)->cookProgress==2.);
  World restored;restored.applyEditEntries(world.getEditEntries());CHECK(restored.restoreFurnaces(state));
  restored.loadChunk(1,1);restored.loadChunk(2,2);CHECK(restored.furnaceAt(p)->fuelRemaining==3.&&restored.furnaceAt(q)->isDefault());
  auto invalid=state;invalid.push_back(state[0]);CHECK(!restored.restoreFurnaces(invalid));CHECK(restored.furnaceAt(p)->cookProgress==2.);
  int items=0,calls=0;auto drop=[&](const glm::vec3& pos,const ItemStack& s){if(pos==glm::vec3(p)){items+=s.count;++calls;}};
  CHECK(restored.setBlock(p.x,p.y,p.z,BlockType::AIR,drop));CHECK(items==4&&calls==3&&!restored.furnaceAt(p));
  CHECK(restored.setBlock(p.x,p.y,p.z,BlockType::AIR,drop));CHECK(calls==3);
  CHECK(restored.setBlock(p.x,p.y,p.z,BlockType::FURNACE));CHECK(restored.furnaceAt(p)->isDefault());
  const auto dir=std::filesystem::temp_directory_path()/"minecraft-furnace-tests";std::filesystem::create_directories(dir);
  const auto path=dir/"save.dat",bad=dir/"bad.dat";
  SaveData data;data.mode=GameMode::Creative;data.health=12;data.hunger=15;data.spawnX=77;data.spawnY=12;data.spawnZ=90;
  data.edits={{17,8,17,BlockType::FURNACE},{33,8,33,BlockType::FURNACE}};
  SaveData::FurnaceData f;f.x=17;f.y=8;f.z=17;f.input=food(FoodType::RAW_BEEF,2);f.output=food(FoodType::COOKED_MUTTON,1);f.fuel.blockType=BlockType::PLANKS;f.fuel.count=2;f.fuelRemaining=3;f.fuelDuration=15;f.cookProgress=2;
  data.furnaces={f};CHECK(SaveLoad::save(data,path));SaveData loaded;CHECK(SaveLoad::load(loaded,path));
  CHECK(loaded.furnaces.size()==1&&loaded.furnaces[0].output.foodType==FoodType::COOKED_MUTTON&&loaded.furnaces[0].cookProgress==2&&loaded.furnaces[0].fuelRemaining==3);
  CHECK(loaded.mode==GameMode::Creative&&loaded.health==12&&loaded.hunger==15&&loaded.spawnX==77&&loaded.spawnZ==90);
  data.furnaces[0].fuelRemaining=0;data.furnaces[0].fuelDuration=0;CHECK(SaveLoad::save(data,path));CHECK(SaveLoad::load(loaded,path)&&loaded.furnaces[0].cookProgress==2);
  data.furnaces[0]=f;
  for(int scenario=0;scenario<9;++scenario){auto broken=data;
    switch(scenario){
      case 0:broken.furnaces.push_back(f);break;
      case 1:broken.furnaces[0].x=999;break;
      case 2:broken.furnaces[0].fuelRemaining=16;break;
      case 3:broken.furnaces[0].fuelDuration=12;break;
      case 4:broken.furnaces[0].cookProgress=10;break;
      case 5:broken.furnaces[0].cookProgress=std::numeric_limits<double>::quiet_NaN();break;
      case 6:broken.furnaces[0].input=food(FoodType::APPLE,1);break;
      case 7:broken.furnaces[0].output=food(FoodType::RAW_BEEF,1);break;
      case 8:broken.furnaces[0].fuel.count=65;break;
    }CHECK(!SaveLoad::save(broken,bad));
  }
  CHECK(SaveLoad::save(data,path));const auto original=bytes(path);
  auto corrupted=original;const double nan=std::numeric_limits<double>::quiet_NaN();std::memcpy(corrupted.data()+corrupted.size()-28-sizeof(double),&nan,sizeof(double));write(bad,corrupted);
  loaded.selectedSlot=8;CHECK(!SaveLoad::load(loaded,bad)&&loaded.selectedSlot==8);
  for(std::size_t missing=1;missing<=30;++missing){corrupted=original;corrupted.resize(corrupted.size()-missing);write(bad,corrupted);CHECK(!SaveLoad::load(loaded,bad)&&loaded.selectedSlot==8);}
  // Encode older versions from an empty current payload, removing only newer fields.
  SaveData empty;CHECK(SaveLoad::save(empty,path));const auto current=bytes(path);
  for(std::uint32_t version=2;version<=7;++version){
    auto old=current;old.resize(old.size()-28); // MCv8 seed, pose and time.
    if(version<7)old.resize(old.size()-4); // Furnace count.
    if(version<4)old.resize(old.size()-4); // Mob count.
    if(version<5)old.erase(old.begin()+140,old.begin()+152); // MCv6 spawn coordinates.
    if(version<3)old.erase(old.begin()+124,old.begin()+140); // Survival fields.
    if(version<6)old.erase(old.begin()+12); // Game mode byte.
    old[3]=static_cast<char>('0'+version);std::memcpy(old.data()+4,&version,sizeof(version));write(path,old);
    CHECK(SaveLoad::load(loaded,path)&&loaded.furnaces.empty()&&loaded.mode==GameMode::Survival);
  }
  std::filesystem::remove_all(dir);return 0;
}
