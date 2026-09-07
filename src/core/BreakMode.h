#pragma once
#include <string>

class BreakModeState {
public:
  void toggle(){m_fullAutoEnabled=!m_fullAutoEnabled;}
  bool fullAutoEnabled()const{return m_fullAutoEnabled;}
  std::string getCurrentBreakModeText()const{return m_fullAutoEnabled?"FULL AUTO":"SINGLE";}
private:
  bool m_fullAutoEnabled=false;
};
