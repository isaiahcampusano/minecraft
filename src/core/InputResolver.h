#pragma once
#include "SettingsState.h"
#include <array>
class InputResolver {
 std::array<bool,1008> down{},blocked{},pressed{},released{};
public:
 void event(int code,bool held){if(!SettingsState::validCode(code))return;const bool was=down[code];down[code]=held;if(held&&!was)pressed[code]=true;if(!held&&was)released[code]=true;if(!held)blocked[code]=false;}
 bool held(const SettingsState& s,KeyAction a,bool flying=false)const{if((a==KeyAction::Sneak&&flying)||(a==KeyAction::FlyDown&&!flying))return false;const int c=s.key(a);return SettingsState::validCode(c)&&down[c]&&!blocked[c];}
 bool justPressed(const SettingsState& s,KeyAction a)const{int c=s.key(a);return SettingsState::validCode(c)&&pressed[c]&&!blocked[c];}
 bool justReleased(const SettingsState& s,KeyAction a)const{int c=s.key(a);return SettingsState::validCode(c)&&released[c];}
 void suppress(){blocked=down;pressed.fill(false);released.fill(false);}
 void clear(){down.fill(false);blocked.fill(false);pressed.fill(false);released.fill(false);}
 bool allReleased()const{for(bool b:down)if(b)return false;return true;}
 void endFrame(){pressed.fill(false);released.fill(false);}
};
