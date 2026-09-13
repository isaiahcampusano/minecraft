#include "WorldRepository.h"
#include "AtomicFile.h"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <random>
#include <sstream>
namespace {
std::uint64_t now(){return static_cast<std::uint64_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));}
bool generatedId(const std::string& id){return id.size()==32&&id.find_first_not_of("0123456789abcdef")==std::string::npos;}
}
bool WorldRepository::safeId(const std::string& id)const{
  if(!generatedId(id))return false;
  std::error_code ec;const auto worlds=root/"worlds",folder=worlds/id;
  const auto base=std::filesystem::weakly_canonical(worlds,ec);if(ec)return false;
  const auto resolved=std::filesystem::weakly_canonical(folder,ec);if(ec||resolved.parent_path()!=base)return false;
  if(std::filesystem::is_symlink(std::filesystem::symlink_status(folder,ec)))return false;
  for(const char* name:{"save.dat","world.meta"}){
    ec.clear();auto file=folder/name;
    if(std::filesystem::is_symlink(std::filesystem::symlink_status(file,ec)))return false;
    ec.clear();if(std::filesystem::weakly_canonical(file,ec).parent_path()!=resolved||ec)return false;
  }
  return true;
}
bool WorldRepository::writeMeta(const WorldInfo& info)const{
  if(!safeId(info.id))return false;
  AtomicFile file(root/"worlds"/info.id/"world.meta");
  file.stream<<"MCWorld 1\n"<<std::quoted(info.name)<<'\n'<<info.seed<<' '<<info.created<<' '<<info.lastPlayed<<' '<<info.imported<<'\n';return file.commit();
}
std::vector<WorldInfo> WorldRepository::list()const{
  std::vector<WorldInfo> result;std::error_code ec;
  std::filesystem::directory_iterator it(root/"worlds",ec),end;
  for(;!ec&&it!=end;it.increment(ec)){
    if(!it->is_directory(ec))continue;
    WorldInfo info;info.id=it->path().filename().string();info.name=info.id;
    if(!safeId(info.id)){info.error="Invalid world directory";result.push_back(info);continue;}
    std::ifstream meta(it->path()/"world.meta");std::string magic,extra;
    if(!std::getline(meta,magic)||magic!="MCWorld 1"||!(meta>>std::quoted(info.name)>>info.seed>>info.created>>info.lastPlayed>>info.imported)||meta>>extra)info.error="Invalid world metadata";
    if(info.name.empty()||info.name.size()>48||info.name.front()==' '||info.name.back()==' '||std::any_of(info.name.begin(),info.name.end(),[](unsigned char c){return c<32||c>126;}))info.error="Invalid world name in metadata";
    SaveData data;std::string error;
    if(!load(info.id,data,error))info.error=error;else{info.seed=data.seed;info.mode=data.mode;}
    result.push_back(info);
  }
  std::sort(result.begin(),result.end(),[](const auto&a,const auto&b){return a.lastPlayed!=b.lastPlayed?a.lastPlayed>b.lastPlayed:a.id<b.id;});return result;
}
bool WorldRepository::create(std::string name,std::uint32_t seed,GameMode mode,WorldInfo& result,std::string& error,bool imported){
  const auto first=name.find_first_not_of(' ');if(first==std::string::npos){error="Enter a world name";return false;}name=name.substr(first,name.find_last_not_of(' ')-first+1);
  if(name.size()>48||std::any_of(name.begin(),name.end(),[](unsigned char c){return c<32||c>126;})){error="Use 1-48 printable characters";return false;}
  std::error_code ec;std::filesystem::create_directories(root/"worlds",ec);if(ec){error="Cannot create worlds folder";return false;}
  std::random_device random;WorldInfo info;info.name=name;info.seed=seed;info.mode=mode;info.created=info.lastPlayed=now();info.imported=imported;
  for(int attempt=0;attempt<16;++attempt){std::ostringstream id;for(int i=0;i<4;++i)id<<std::hex<<std::setw(8)<<std::setfill('0')<<static_cast<std::uint32_t>(random());info.id=id.str();
    if(safeId(info.id)&&std::filesystem::create_directory(root/"worlds"/info.id,ec))break;
    if(attempt==15||ec){error="Cannot create world folder";return false;}}
  SaveData data;data.seed=seed;data.mode=mode;
  if(!SaveLoad::save(data,root/"worlds"/info.id/"save.dat")||!writeMeta(info)){error="Cannot save new world";return false;}
  result=info;return true;
}
bool WorldRepository::load(const std::string& id,SaveData& data,std::string& error)const{
  if(!safeId(id)){error="Invalid world path";return false;}
  if(!SaveLoad::load(data,root/"worlds"/id/"save.dat")){error="World save is missing or invalid";return false;}return true;
}
bool WorldRepository::save(WorldInfo& info,const SaveData& data,std::string& error){
  if(!safeId(info.id)||!SaveLoad::save(data,root/"worlds"/info.id/"save.dat")){error="Could not save world. Your session is still open.";return false;}
  info.seed=data.seed;info.mode=data.mode;info.lastPlayed=now();if(!writeMeta(info)){error="Could not update world metadata";return false;}return true;
}
bool WorldRepository::remove(const std::string& id,std::string& error){
  if(!safeId(id)){error="Invalid world path";return false;}
  std::error_code ec;std::filesystem::remove_all(root/"worlds"/id,ec);if(ec){error="Could not delete world";return false;}return true;
}
bool WorldRepository::importLegacy(std::string& notice){
  std::error_code ec;const auto legacy=root/"save.dat",marker=root/"legacy-import.txt";
  if(std::filesystem::exists(marker,ec)||!std::filesystem::exists(legacy,ec))return true;
  SaveData data;if(!SaveLoad::load(data,legacy)){notice="Legacy save is invalid; original preserved";return false;}
  // The imported flag also recovers a crash between creation and writing the marker.
  for(const auto& info:list())if(info.imported&&info.error.empty()){AtomicFile file(marker);file.stream<<info.id;return file.commit();}
  WorldInfo info;if(!create("Imported World",data.seed,data.mode,info,notice))return false;
  AtomicFile copy(root/"worlds"/info.id/"save.dat");std::ifstream original(legacy,std::ios::binary);copy.stream<<original.rdbuf();if(!original||!copy.commit()){notice="Could not copy legacy save";return false;}
  info.imported=true;if(!writeMeta(info)){notice="Could not finish legacy import";return false;}
  AtomicFile file(marker);file.stream<<info.id;if(!file.commit()){notice="Could not record legacy import";return false;}notice="Imported your existing world; original preserved";return true;
}
