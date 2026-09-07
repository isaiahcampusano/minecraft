#include "core/SaveLoad.h"
#include "player/Inventory.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdint>

namespace {
int fail(const char* message){std::cerr<<message<<'\n';return 1;}
SaveData::SlotData blockSlot(BlockType type,std::uint8_t count){SaveData::SlotData slot;slot.blockType=type;slot.count=count;return slot;}
SaveData::SlotData materialSlot(MaterialType type,std::uint8_t count){SaveData::SlotData slot;slot.kind=ItemKind::MATERIAL;slot.materialType=type;slot.count=count;return slot;}
SaveData::SlotData toolSlot(ToolKind kind,ToolTier tier,std::uint16_t durability){SaveData::SlotData slot;slot.kind=ItemKind::TOOL;slot.toolKind=kind;slot.toolTier=tier;slot.count=1;slot.durability=durability;return slot;}
SaveData::SlotData foodSlot(FoodType type,std::uint8_t count){SaveData::SlotData slot;slot.kind=ItemKind::FOOD;slot.foodType=type;slot.count=count;return slot;}
bool sameSlot(const SaveData::SlotData& a,const SaveData::SlotData& b){return a.kind==b.kind&&a.blockType==b.blockType&&a.materialType==b.materialType&&a.toolKind==b.toolKind&&a.toolTier==b.toolTier&&a.foodType==b.foodType&&a.count==b.count&&a.durability==b.durability;}
template<typename T>void raw(std::ostream& out,const T& value){out.write(reinterpret_cast<const char*>(&value),sizeof(value));}
void emptyV2Slot(std::ostream& out){const std::uint8_t kind=0,type=0,count=0;raw(out,kind);raw(out,type);raw(out,count);}
void writeV2Save(const std::filesystem::path& path){std::ofstream out(path,std::ios::binary|std::ios::trunc);out.write("MCv2",4);const std::uint32_t version=2;const std::int32_t selected=4;raw(out,version);raw(out,selected);emptyV2Slot(out);for(int i=0;i<9;++i){if(i==4){const std::uint8_t kind=0,type=static_cast<std::uint8_t>(BlockType::DIRT),count=12;raw(out,kind);raw(out,type);raw(out,count);}else emptyV2Slot(out);}for(int i=0;i<27;++i)emptyV2Slot(out);const std::uint32_t edits=1;raw(out,edits);const std::int32_t x=9,y=8,z=7;const std::uint8_t type=static_cast<std::uint8_t>(BlockType::GLASS);raw(out,x);raw(out,y);raw(out,z);raw(out,type);}
void writeV3Save(const std::filesystem::path& path,std::int32_t health,std::int32_t hunger,float saturation,float exhaustion){std::ofstream out(path,std::ios::binary|std::ios::trunc);out.write("MCv3",4);const std::uint32_t version=3;const std::int32_t selected=0;raw(out,version);raw(out,selected);for(int i=0;i<37;++i)emptyV2Slot(out);raw(out,health);raw(out,hunger);raw(out,saturation);raw(out,exhaustion);const std::uint32_t edits=0;raw(out,edits);}
}

int main(){
  const auto directory=std::filesystem::temp_directory_path()/"minecraftclone-save-tests",path=directory/"roundtrip.dat",corruptPath=directory/"corrupt.dat";std::error_code error;std::filesystem::create_directories(directory,error);if(error)return fail("could not create temporary save directory");
  SaveData source;source.selectedSlot=7;source.cursorStack=materialSlot(MaterialType::STICK,64);
  for(std::size_t i=0;i<source.hotbar.size();++i)source.hotbar[i]=blockSlot(static_cast<BlockType>(1+i%(BLOCK_TYPE_COUNT-1)),static_cast<std::uint8_t>(i+1));
  for(std::size_t i=0;i<source.backpack.size();++i)source.backpack[i]=blockSlot(static_cast<BlockType>(1+i%(BLOCK_TYPE_COUNT-1)),static_cast<std::uint8_t>(Inventory::MAX_STACK_SIZE-i));
  source.hotbar[0]=toolSlot(ToolKind::PICKAXE,ToolTier::WOOD,17);source.hotbar[1]=toolSlot(ToolKind::AXE,ToolTier::STONE,129);source.hotbar[2]=foodSlot(FoodType::APPLE,11);source.backpack[3]=materialSlot(MaterialType::STICK,12);source.health=13;source.hunger=16;source.saturation=4.5f;source.exhaustion=3.25f;
  for(int i=0;i<1000;++i)source.edits.push_back({i%1000,(i*7)%256,(i*13)%1000,i%5==0?BlockType::AIR:BlockType::GLASS});
  if(!SaveLoad::save(source,path)||!std::filesystem::exists(path)||std::filesystem::file_size(path)==0)return fail("save file was not written");
  SaveData loaded;if(!SaveLoad::load(loaded,path))return fail("valid save file did not load");
  if(loaded.selectedSlot!=source.selectedSlot||!sameSlot(loaded.cursorStack,source.cursorStack)||loaded.edits.size()!=source.edits.size()||loaded.health!=13||loaded.hunger!=16||loaded.saturation!=4.5f||loaded.exhaustion!=3.25f)return fail("top-level save data did not round-trip");
  for(std::size_t i=0;i<source.hotbar.size();++i)if(!sameSlot(loaded.hotbar[i],source.hotbar[i]))return fail("hotbar did not round-trip");
  for(std::size_t i=0;i<source.backpack.size();++i)if(!sameSlot(loaded.backpack[i],source.backpack[i]))return fail("backpack did not round-trip");
  for(std::size_t i=0;i<source.edits.size();++i){const auto& a=loaded.edits[i];const auto& b=source.edits[i];if(a.x!=b.x||a.y!=b.y||a.z!=b.z||a.type!=b.type)return fail("world edits did not round-trip");}
  SaveData unchanged;unchanged.selectedSlot=3;if(SaveLoad::load(unchanged,directory/"missing.dat")||unchanged.selectedSlot!=3)return fail("missing save did not preserve destination state");
  {std::ofstream corrupt(corruptPath,std::ios::binary);corrupt<<"bad save";}if(SaveLoad::load(unchanged,corruptPath)||unchanged.selectedSlot!=3)return fail("corrupt save was not rejected atomically");
  source.cursorStack=blockSlot(BlockType::AIR,4);if(SaveLoad::save(source,directory/"invalid.dat"))return fail("invalid stack was serialized");
  source.cursorStack=materialSlot(MaterialType::STICK,1);source.saturation=17.f;if(SaveLoad::save(source,directory/"invalid-survival.dat"))return fail("invalid survival range was serialized");source.saturation=4.5f;
  const auto v2Path=directory/"migration-v2.dat";writeV2Save(v2Path);SaveData migrated;if(!SaveLoad::load(migrated,v2Path)||migrated.selectedSlot!=4||migrated.hotbar[4].blockType!=BlockType::DIRT||migrated.hotbar[4].count!=12||migrated.edits.size()!=1||migrated.health!=20||migrated.hunger!=20||migrated.saturation!=5.f||migrated.exhaustion!=0.f)return fail("MCv2 migration failed");
  const auto invalidRangePath=directory/"invalid-range.dat";writeV3Save(invalidRangePath,20,4,5.f,0.f);unchanged.selectedSlot=3;if(SaveLoad::load(unchanged,invalidRangePath)||unchanged.selectedSlot!=3)return fail("invalid MCv3 range was not rejected atomically");
  {std::ofstream old(corruptPath,std::ios::binary|std::ios::trunc);old.write("MCv1",4);const std::uint32_t version=1;old.write(reinterpret_cast<const char*>(&version),sizeof(version));}if(SaveLoad::load(unchanged,corruptPath))return fail("MCv1 save was not rejected");
#ifdef _WIN32
  if(SaveLoad::getSavePath().filename()!="save.dat"||SaveLoad::getSavePath().parent_path().filename()!="MinecraftClone")return fail("Windows save path did not use APPDATA/MinecraftClone/save.dat");
#else
  if(SaveLoad::getSavePath().filename()!="save.dat"||SaveLoad::getSavePath().parent_path().filename()!=".minecraftclone")return fail("Unix save path did not use ~/.minecraftclone/save.dat");
#endif
  std::filesystem::remove_all(directory,error);return 0;
}
