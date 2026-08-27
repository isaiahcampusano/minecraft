#include "SaveLoad.h"
#include "../player/Inventory.h"
#include "../player/ToolRegistry.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <utility>

namespace {
constexpr std::array<char,4> MAGIC={'M','C','v','2'};
constexpr std::uint32_t MAX_EDIT_COUNT=10000000;
constexpr int WORLD_SIZE=1000,WORLD_HEIGHT=256;

template<typename T> bool writeValue(std::ostream& out,const T& value){out.write(reinterpret_cast<const char*>(&value),sizeof(value));return static_cast<bool>(out);}
template<typename T> bool readValue(std::istream& in,T& value){in.read(reinterpret_cast<char*>(&value),sizeof(value));return static_cast<bool>(in);}
bool validType(BlockType type){return static_cast<std::size_t>(type)<BLOCK_TYPE_COUNT;}
bool validSlot(const SaveData::SlotData& slot){
  if(static_cast<std::size_t>(slot.kind)>static_cast<std::size_t>(ItemKind::TOOL))return false;
  if(slot.kind==ItemKind::BLOCK)return validType(slot.blockType)&&slot.count<=Inventory::MAX_STACK_SIZE&&slot.durability==0&&((slot.count==0)==(slot.blockType==BlockType::AIR));
  if(slot.kind==ItemKind::MATERIAL)return static_cast<std::size_t>(slot.materialType)<static_cast<std::size_t>(MaterialType::COUNT)&&slot.count>0&&slot.count<=Inventory::MAX_STACK_SIZE&&slot.durability==0;
  return static_cast<std::size_t>(slot.toolKind)<static_cast<std::size_t>(ToolKind::COUNT)&&static_cast<std::size_t>(slot.toolTier)<static_cast<std::size_t>(ToolTier::COUNT)&&slot.count==1&&slot.durability>0&&slot.durability<=maxToolDurability(slot.toolTier);
}
bool writeSlot(std::ostream& out,const SaveData::SlotData& slot){
  const auto kind=static_cast<std::uint8_t>(slot.kind);if(!writeValue(out,kind))return false;
  if(slot.kind==ItemKind::BLOCK){const auto type=static_cast<std::uint8_t>(slot.blockType);return writeValue(out,type)&&writeValue(out,slot.count);}
  if(slot.kind==ItemKind::MATERIAL){const auto type=static_cast<std::uint8_t>(slot.materialType);return writeValue(out,type)&&writeValue(out,slot.count);}
  const auto toolKind=static_cast<std::uint8_t>(slot.toolKind),tier=static_cast<std::uint8_t>(slot.toolTier);return writeValue(out,toolKind)&&writeValue(out,tier)&&writeValue(out,slot.count)&&writeValue(out,slot.durability);
}
bool readSlot(std::istream& in,SaveData::SlotData& slot){
  std::uint8_t kind=0,type=0;if(!readValue(in,kind))return false;slot.kind=static_cast<ItemKind>(kind);
  if(slot.kind==ItemKind::BLOCK){if(!readValue(in,type)||!readValue(in,slot.count))return false;slot.blockType=static_cast<BlockType>(type);}
  else if(slot.kind==ItemKind::MATERIAL){if(!readValue(in,type)||!readValue(in,slot.count))return false;slot.materialType=static_cast<MaterialType>(type);}
  else if(slot.kind==ItemKind::TOOL){std::uint8_t tier=0;if(!readValue(in,type)||!readValue(in,tier)||!readValue(in,slot.count)||!readValue(in,slot.durability))return false;slot.toolKind=static_cast<ToolKind>(type);slot.toolTier=static_cast<ToolTier>(tier);}
  else return false;
  return validSlot(slot);
}
bool validEdit(const SaveData::EditData& edit){return edit.x>=0&&edit.x<WORLD_SIZE&&edit.z>=0&&edit.z<WORLD_SIZE&&edit.y>=0&&edit.y<WORLD_HEIGHT&&validType(edit.type);}
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
  if(data.selectedSlot<0||data.selectedSlot>=Inventory::HOTBAR_SLOTS||!validSlot(data.cursorStack)||data.edits.size()>std::numeric_limits<std::uint32_t>::max())return false;
  if(!std::all_of(data.hotbar.begin(),data.hotbar.end(),validSlot)||!std::all_of(data.backpack.begin(),data.backpack.end(),validSlot)||!std::all_of(data.edits.begin(),data.edits.end(),validEdit))return false;
  std::error_code error;if(!path.parent_path().empty())std::filesystem::create_directories(path.parent_path(),error);if(error)return false;
  std::ofstream out(path,std::ios::binary|std::ios::trunc);if(!out)return false;
  out.write(MAGIC.data(),MAGIC.size());const std::uint32_t version=VERSION,editCount=static_cast<std::uint32_t>(data.edits.size());
  if(!out||!writeValue(out,version)||!writeValue(out,data.selectedSlot)||!writeSlot(out,data.cursorStack))return false;
  for(const auto& slot:data.hotbar)if(!writeSlot(out,slot))return false;
  for(const auto& slot:data.backpack)if(!writeSlot(out,slot))return false;
  if(!writeValue(out,editCount))return false;
  for(const auto& edit:data.edits){const std::int32_t x=edit.x,y=edit.y,z=edit.z;const auto type=static_cast<std::uint8_t>(edit.type);if(!writeValue(out,x)||!writeValue(out,y)||!writeValue(out,z)||!writeValue(out,type))return false;}
  out.flush();return static_cast<bool>(out);
}

bool SaveLoad::load(SaveData& out){return load(out,getSavePath());}
bool SaveLoad::load(SaveData& out,const std::filesystem::path& path){
  std::ifstream in(path,std::ios::binary);if(!in)return false;SaveData data;std::array<char,4> magic{};in.read(magic.data(),magic.size());std::uint32_t version=0;
  if(!in||magic!=MAGIC||!readValue(in,version)||version!=VERSION||!readValue(in,data.selectedSlot)||data.selectedSlot<0||data.selectedSlot>=Inventory::HOTBAR_SLOTS||!readSlot(in,data.cursorStack))return false;
  for(auto& slot:data.hotbar)if(!readSlot(in,slot))return false;
  for(auto& slot:data.backpack)if(!readSlot(in,slot))return false;
  std::uint32_t editCount=0;if(!readValue(in,editCount)||editCount>MAX_EDIT_COUNT)return false;data.edits.reserve(editCount);
  for(std::uint32_t i=0;i<editCount;++i){std::int32_t x=0,y=0,z=0;std::uint8_t type=0;if(!readValue(in,x)||!readValue(in,y)||!readValue(in,z)||!readValue(in,type))return false;SaveData::EditData edit{x,y,z,static_cast<BlockType>(type)};if(!validEdit(edit))return false;data.edits.push_back(edit);}
  if(in.peek()!=std::char_traits<char>::eof())return false;
  out=std::move(data);return true;
}
