#pragma once
#include "SaveLoad.h"
#include <string>
#include <vector>
struct WorldInfo {
  std::string id,name,error;std::uint32_t seed=0;GameMode mode=GameMode::Survival;
  std::uint64_t created=0,lastPlayed=0;bool imported=false;
};
class WorldRepository {
  std::filesystem::path root;
  bool safeId(const std::string& id)const;
  bool writeMeta(const WorldInfo&)const;
public:
  explicit WorldRepository(std::filesystem::path base=SaveLoad::getSavePath().parent_path()):root(std::move(base)){}
  std::vector<WorldInfo> list()const;
  bool create(std::string name,std::uint32_t seed,GameMode mode,WorldInfo& result,std::string& error,bool imported=false);
  bool load(const std::string& id,SaveData& data,std::string& error)const;
  bool save(WorldInfo& info,const SaveData& data,std::string& error);
  bool remove(const std::string& id,std::string& error);
  bool importLegacy(std::string& notice);
};
