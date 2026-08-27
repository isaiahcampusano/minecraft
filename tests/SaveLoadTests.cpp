#include "core/SaveLoad.h"
#include "player/Inventory.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace {
int fail(const char* message){std::cerr<<message<<'\n';return 1;}
bool sameSlot(const SaveData::SlotData& a,const SaveData::SlotData& b){return a.type==b.type&&a.count==b.count;}
}

int main(){
  const auto directory=std::filesystem::temp_directory_path()/"minecraftclone-save-tests",path=directory/"roundtrip.dat",corruptPath=directory/"corrupt.dat";std::error_code error;std::filesystem::create_directories(directory,error);if(error)return fail("could not create temporary save directory");
  SaveData source;source.selectedSlot=7;source.cursorStack={BlockType::LEAVES,64};
  for(std::size_t i=0;i<source.hotbar.size();++i)source.hotbar[i]={static_cast<BlockType>(1+i%BLOCK_TYPE_COUNT),static_cast<std::uint8_t>(i+1)};
  for(std::size_t i=0;i<source.backpack.size();++i)source.backpack[i]={static_cast<BlockType>(1+i%(BLOCK_TYPE_COUNT-1)),static_cast<std::uint8_t>(Inventory::MAX_STACK_SIZE-i)};
  for(int i=0;i<1000;++i)source.edits.push_back({i%1000,(i*7)%256,(i*13)%1000,i%5==0?BlockType::AIR:BlockType::GLASS});
  if(!SaveLoad::save(source,path)||!std::filesystem::exists(path)||std::filesystem::file_size(path)==0)return fail("save file was not written");
  if(std::filesystem::file_size(path)!=13090)return fail("binary save layout size did not match version 1 format");
  SaveData loaded;if(!SaveLoad::load(loaded,path))return fail("valid save file did not load");
  if(loaded.selectedSlot!=source.selectedSlot||!sameSlot(loaded.cursorStack,source.cursorStack)||loaded.edits.size()!=source.edits.size())return fail("top-level save data did not round-trip");
  for(std::size_t i=0;i<source.hotbar.size();++i)if(!sameSlot(loaded.hotbar[i],source.hotbar[i]))return fail("hotbar did not round-trip");
  for(std::size_t i=0;i<source.backpack.size();++i)if(!sameSlot(loaded.backpack[i],source.backpack[i]))return fail("backpack did not round-trip");
  for(std::size_t i=0;i<source.edits.size();++i){const auto& a=loaded.edits[i];const auto& b=source.edits[i];if(a.x!=b.x||a.y!=b.y||a.z!=b.z||a.type!=b.type)return fail("world edits did not round-trip");}
  SaveData unchanged;unchanged.selectedSlot=3;if(SaveLoad::load(unchanged,directory/"missing.dat")||unchanged.selectedSlot!=3)return fail("missing save did not preserve destination state");
  {std::ofstream corrupt(corruptPath,std::ios::binary);corrupt<<"bad save";}if(SaveLoad::load(unchanged,corruptPath)||unchanged.selectedSlot!=3)return fail("corrupt save was not rejected atomically");
  source.cursorStack={BlockType::AIR,4};if(SaveLoad::save(source,directory/"invalid.dat"))return fail("invalid stack was serialized");
#ifdef _WIN32
  if(SaveLoad::getSavePath().filename()!="save.dat"||SaveLoad::getSavePath().parent_path().filename()!="MinecraftClone")return fail("Windows save path did not use APPDATA/MinecraftClone/save.dat");
#else
  if(SaveLoad::getSavePath().filename()!="save.dat"||SaveLoad::getSavePath().parent_path().filename()!=".minecraftclone")return fail("Unix save path did not use ~/.minecraftclone/save.dat");
#endif
  std::filesystem::remove_all(directory,error);return 0;
}
