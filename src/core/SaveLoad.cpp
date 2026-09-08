#include "SaveLoad.h"
#include "../player/Inventory.h"
#include "../player/ToolRegistry.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <limits>
#include <utility>

namespace {
constexpr std::array<char,4> MAGIC_V2={'M','C','v','2'},MAGIC_V3={'M','C','v','3'},MAGIC_V4={'M','C','v','4'};
constexpr std::uint32_t MAX_EDIT_COUNT=10000000,MAX_MOB_COUNT=32;
constexpr int WORLD_SIZE=1000,WORLD_HEIGHT=256;

template<typename T> bool writeValue(std::ostream& out,const T& value){out.write(reinterpret_cast<const char*>(&value),sizeof(value));return static_cast<bool>(out);}
template<typename T> bool readValue(std::istream& in,T& value){in.read(reinterpret_cast<char*>(&value),sizeof(value));return static_cast<bool>(in);}
bool validType(BlockType type){return static_cast<std::size_t>(type)<BLOCK_TYPE_COUNT;}
bool validSlot(const SaveData::SlotData& slot,std::uint32_t version=SaveLoad::VERSION){
  if(static_cast<std::size_t>(slot.kind)>static_cast<std::size_t>(version>=3?ItemKind::FOOD:ItemKind::TOOL))return false;
  if(slot.kind==ItemKind::BLOCK)return validType(slot.blockType)&&slot.count<=Inventory::MAX_STACK_SIZE&&slot.durability==0&&((slot.count==0)==(slot.blockType==BlockType::AIR));
  if(slot.kind==ItemKind::MATERIAL){const std::size_t count=version>=4?static_cast<std::size_t>(MaterialType::COUNT):1u;return static_cast<std::size_t>(slot.materialType)<count&&slot.count>0&&slot.count<=Inventory::MAX_STACK_SIZE&&slot.durability==0;}
  if(slot.kind==ItemKind::FOOD){const std::size_t count=version>=4?static_cast<std::size_t>(FoodType::COUNT):1u;return static_cast<std::size_t>(slot.foodType)<count&&slot.count>0&&slot.count<=Inventory::MAX_STACK_SIZE&&slot.durability==0;}
  return static_cast<std::size_t>(slot.toolKind)<static_cast<std::size_t>(ToolKind::COUNT)&&static_cast<std::size_t>(slot.toolTier)<static_cast<std::size_t>(ToolTier::COUNT)&&slot.count==1&&slot.durability>0&&slot.durability<=maxToolDurability(slot.toolTier);
}
bool writeSlot(std::ostream& out,const SaveData::SlotData& slot){
  const auto kind=static_cast<std::uint8_t>(slot.kind);if(!writeValue(out,kind))return false;
  if(slot.kind==ItemKind::BLOCK){const auto type=static_cast<std::uint8_t>(slot.blockType);return writeValue(out,type)&&writeValue(out,slot.count);}
  if(slot.kind==ItemKind::MATERIAL){const auto type=static_cast<std::uint8_t>(slot.materialType);return writeValue(out,type)&&writeValue(out,slot.count);}
  if(slot.kind==ItemKind::FOOD){const auto type=static_cast<std::uint8_t>(slot.foodType);return writeValue(out,type)&&writeValue(out,slot.count);}
  const auto toolKind=static_cast<std::uint8_t>(slot.toolKind),tier=static_cast<std::uint8_t>(slot.toolTier);return writeValue(out,toolKind)&&writeValue(out,tier)&&writeValue(out,slot.count)&&writeValue(out,slot.durability);
}
bool readSlot(std::istream& in,SaveData::SlotData& slot,std::uint32_t version){
  std::uint8_t kind=0,type=0;if(!readValue(in,kind))return false;slot.kind=static_cast<ItemKind>(kind);
  if(slot.kind==ItemKind::BLOCK){if(!readValue(in,type)||!readValue(in,slot.count))return false;slot.blockType=static_cast<BlockType>(type);}
  else if(slot.kind==ItemKind::MATERIAL){if(!readValue(in,type)||!readValue(in,slot.count))return false;slot.materialType=static_cast<MaterialType>(type);}
  else if(slot.kind==ItemKind::TOOL){std::uint8_t tier=0;if(!readValue(in,type)||!readValue(in,tier)||!readValue(in,slot.count)||!readValue(in,slot.durability))return false;slot.toolKind=static_cast<ToolKind>(type);slot.toolTier=static_cast<ToolTier>(tier);}
  else if(slot.kind==ItemKind::FOOD&&version>=3){if(!readValue(in,type)||!readValue(in,slot.count))return false;slot.foodType=static_cast<FoodType>(type);}
  else return false;
  return validSlot(slot,version);
}
bool validEdit(const SaveData::EditData& edit){return edit.x>=0&&edit.x<WORLD_SIZE&&edit.z>=0&&edit.z<WORLD_SIZE&&edit.y>=0&&edit.y<WORLD_HEIGHT&&validType(edit.type);}
bool validSurvival(const SaveData& data){return data.health>=0&&data.health<=20&&data.hunger>=0&&data.hunger<=20&&std::isfinite(data.saturation)&&data.saturation>=0&&data.saturation<=static_cast<float>(data.hunger)&&std::isfinite(data.exhaustion)&&data.exhaustion>=0&&data.exhaustion<4.f;}
bool validMob(const MobData& mob){return mob.id>0&&static_cast<std::size_t>(mob.type)<static_cast<std::size_t>(MobType::COUNT)&&std::isfinite(mob.x)&&std::isfinite(mob.y)&&std::isfinite(mob.z)&&std::isfinite(mob.yaw)&&std::isfinite(mob.age)&&std::isfinite(mob.grazeCooldown)&&mob.x>=0&&mob.x<1000&&mob.z>=0&&mob.z<1000&&mob.y>=0&&mob.y<256&&mob.health>0&&mob.health<=mobMaxHealth(mob.type)&&mob.age>=-1200.f&&mob.age<=0&&mob.grazeCooldown>=0&&mob.grazeCooldown<=60.f;}
bool validMobs(const std::vector<MobData>& mobs){if(mobs.size()>MAX_MOB_COUNT)return false;std::vector<MobId> ids;ids.reserve(mobs.size());for(const auto& mob:mobs){if(!validMob(mob)||std::find(ids.begin(),ids.end(),mob.id)!=ids.end())return false;ids.push_back(mob.id);}return true;}
}

std::filesystem::path SaveLoad::getSavePath(){
#ifdef _WIN32
  const char* root=std::getenv("APPDATA");
  return(root&&*root?std::filesystem::path(root):std::filesystem::current_path())/"MinecraftClone"/"save.dat";
#else
  const char* root=std::getenv("HOME");
  return(root&&*root?std::filesystem::path(root):std::filesystem::current_path())/".minecraftclone"/"save.dat";
#endif
}

bool SaveLoad::save(const SaveData& data){return save(data,getSavePath());}
bool SaveLoad::save(const SaveData& data,const std::filesystem::path& path){
  if(data.selectedSlot<0||data.selectedSlot>=Inventory::HOTBAR_SLOTS||!validSlot(data.cursorStack)||!validSurvival(data)||data.edits.size()>std::numeric_limits<std::uint32_t>::max()||!validMobs(data.mobs))return false;
  const auto validCurrentSlot=[](const SaveData::SlotData& slot){return validSlot(slot);};
  if(!std::all_of(data.hotbar.begin(),data.hotbar.end(),validCurrentSlot)||!std::all_of(data.backpack.begin(),data.backpack.end(),validCurrentSlot)||!std::all_of(data.edits.begin(),data.edits.end(),validEdit))return false;
  std::error_code error;if(!path.parent_path().empty())std::filesystem::create_directories(path.parent_path(),error);if(error)return false;
  std::ofstream out(path,std::ios::binary|std::ios::trunc);if(!out)return false;
  out.write(MAGIC_V4.data(),MAGIC_V4.size());const std::uint32_t version=VERSION,editCount=static_cast<std::uint32_t>(data.edits.size()),mobCount=static_cast<std::uint32_t>(data.mobs.size());
  if(!out||!writeValue(out,version)||!writeValue(out,data.selectedSlot)||!writeSlot(out,data.cursorStack))return false;
  for(const auto& slot:data.hotbar)if(!writeSlot(out,slot))return false;
  for(const auto& slot:data.backpack)if(!writeSlot(out,slot))return false;
  if(!writeValue(out,data.health)||!writeValue(out,data.hunger)||!writeValue(out,data.saturation)||!writeValue(out,data.exhaustion))return false;
  if(!writeValue(out,editCount))return false;
  for(const auto& edit:data.edits){const std::int32_t x=edit.x,y=edit.y,z=edit.z;const auto type=static_cast<std::uint8_t>(edit.type);if(!writeValue(out,x)||!writeValue(out,y)||!writeValue(out,z)||!writeValue(out,type))return false;}
  if(!writeValue(out,mobCount))return false;
  for(const auto& mob:data.mobs){const auto type=static_cast<std::uint8_t>(mob.type);if(!writeValue(out,mob.id)||!writeValue(out,type)||!writeValue(out,mob.x)||!writeValue(out,mob.y)||!writeValue(out,mob.z)||!writeValue(out,mob.yaw)||!writeValue(out,mob.health)||!writeValue(out,mob.age)||!writeValue(out,mob.grazeCooldown))return false;}
  out.flush();return static_cast<bool>(out);
}

bool SaveLoad::load(SaveData& out){return load(out,getSavePath());}
bool SaveLoad::load(SaveData& out,const std::filesystem::path& path){
  std::ifstream in(path,std::ios::binary);if(!in)return false;SaveData data;std::array<char,4> magic{};in.read(magic.data(),magic.size());std::uint32_t version=0;
  if(!in||!readValue(in,version)||!((magic==MAGIC_V2&&version==2)||(magic==MAGIC_V3&&version==3)||(magic==MAGIC_V4&&version==VERSION))||!readValue(in,data.selectedSlot)||data.selectedSlot<0||data.selectedSlot>=Inventory::HOTBAR_SLOTS||!readSlot(in,data.cursorStack,version))return false;
  for(auto& slot:data.hotbar)if(!readSlot(in,slot,version))return false;
  for(auto& slot:data.backpack)if(!readSlot(in,slot,version))return false;
  if(version>=3&&(!readValue(in,data.health)||!readValue(in,data.hunger)||!readValue(in,data.saturation)||!readValue(in,data.exhaustion)||!validSurvival(data)))return false;
  std::uint32_t editCount=0;if(!readValue(in,editCount)||editCount>MAX_EDIT_COUNT)return false;data.edits.reserve(editCount);
  for(std::uint32_t i=0;i<editCount;++i){std::int32_t x=0,y=0,z=0;std::uint8_t type=0;if(!readValue(in,x)||!readValue(in,y)||!readValue(in,z)||!readValue(in,type))return false;SaveData::EditData edit{x,y,z,static_cast<BlockType>(type)};if(!validEdit(edit))return false;data.edits.push_back(edit);}
  if(version>=4){std::uint32_t mobCount=0;if(!readValue(in,mobCount)||mobCount>MAX_MOB_COUNT)return false;data.mobs.reserve(mobCount);for(std::uint32_t i=0;i<mobCount;++i){MobData mob;std::uint8_t type=0;if(!readValue(in,mob.id)||!readValue(in,type)||!readValue(in,mob.x)||!readValue(in,mob.y)||!readValue(in,mob.z)||!readValue(in,mob.yaw)||!readValue(in,mob.health)||!readValue(in,mob.age)||!readValue(in,mob.grazeCooldown))return false;mob.type=static_cast<MobType>(type);data.mobs.push_back(mob);}if(!validMobs(data.mobs))return false;}
  if(in.peek()!=std::char_traits<char>::eof())return false;
  out=std::move(data);return true;
}
