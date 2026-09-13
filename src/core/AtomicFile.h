#pragma once
#include <filesystem>
#include <fstream>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
class AtomicFile {
  std::filesystem::path target,temp;
  bool finished=false;
public:
  std::ofstream stream;
  explicit AtomicFile(const std::filesystem::path& path):target(path),temp(path.string()+".tmp"),stream(temp,std::ios::binary|std::ios::trunc){}
  ~AtomicFile(){if(!finished){stream.close();std::error_code ec;std::filesystem::remove(temp,ec);}}
  bool commit(){stream.flush();if(!stream)return false;stream.close();if(stream.fail())return false;
#ifdef _WIN32
    if(!MoveFileExW(temp.c_str(),target.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))return false;
#else
    std::error_code ec;std::filesystem::rename(temp,target,ec);if(ec)return false;
#endif
    finished=true;return true;
  }
};
