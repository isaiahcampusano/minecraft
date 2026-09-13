#include "SettingsState.h"
#include "SaveLoad.h"
#include "AtomicFile.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <set>
namespace {
constexpr int defaults[]={87,83,65,68,32,341,340,69,70,294,49,50,51,52,53,54,55,56,57,256,1000,1001,1002,81,341,84,290,67};
constexpr const char* names[]={"move_forward","move_backward","move_left","move_right","jump","fly_down","sprint","inventory","toggle_fly","toggle_perspective","hotbar_1","hotbar_2","hotbar_3","hotbar_4","hotbar_5","hotbar_6","hotbar_7","hotbar_8","hotbar_9","pause","attack","use","pick_block","drop_item","sneak","open_chat","toggle_hud","zoom"};
}
SettingsState::SettingsState(){reset();}
void SettingsState::reset(){viewDistance=4;baseFov=70;mouseSensitivity=.15f;vsync=true;invertY=false;rawMouseInput=true;defaultMode=GameMode::Survival;notice.clear();resetBindings();}
void SettingsState::resetBindings(){std::copy(std::begin(defaults),std::end(defaults),keybindings.begin());}
int SettingsState::defaultBinding(KeyAction a){return defaults[static_cast<std::size_t>(a)];}
void SettingsState::setViewDistance(int value){viewDistance=std::clamp(value,2,8);}
int SettingsState::key(KeyAction a)const{return keybindings[static_cast<std::size_t>(a)];}
int& SettingsState::key(KeyAction a){return keybindings[static_cast<std::size_t>(a)];}
const char* SettingsState::actionName(KeyAction a){return static_cast<std::size_t>(a)<ACTION_COUNT?names[static_cast<std::size_t>(a)]:"";}
bool SettingsState::validCode(int c){return c==32||c==39||(c>=44&&c<=57)||c==59||c==61||(c>=65&&c<=93)||c==96||c==161||c==162||(c>=256&&c<=269)||(c>=280&&c<=284)||(c>=290&&c<=314)||(c>=320&&c<=336)||(c>=340&&c<=GLFW_KEY_LAST)||(c>=MOUSE_OFFSET&&c<=MOUSE_OFFSET+GLFW_MOUSE_BUTTON_LAST);}
bool SettingsState::contextsOverlap(KeyAction a,KeyAction b){return !((a==KeyAction::Sneak&&b==KeyAction::FlyDown)||(b==KeyAction::Sneak&&a==KeyAction::FlyDown));}
std::vector<KeyAction> SettingsState::conflicts(KeyAction action,int code)const{
 std::vector<KeyAction> result;for(std::size_t i=0;i<ACTION_COUNT;++i){auto other=static_cast<KeyAction>(i);if(other!=action&&contextsOverlap(action,other)&&keybindings[i]==code)result.push_back(other);}return result;
}
bool SettingsState::assign(KeyAction action,int code,bool swap){
 if(!validCode(code)||(code==GLFW_KEY_ESCAPE&&action!=KeyAction::Pause))return false;
 auto collision=conflicts(action,code);if(collision.size()>1||(!collision.empty()&&!swap))return false;
 auto candidate=*this;if(!collision.empty())candidate.key(collision.front())=key(action);candidate.key(action)=code;
 if(!candidate.valid())return false;
 keybindings=candidate.keybindings;return true;
}
bool SettingsState::valid()const{
 if(viewDistance<2||viewDistance>8||!std::isfinite(baseFov)||baseFov<30||baseFov>90||!std::isfinite(mouseSensitivity)||mouseSensitivity<.01f||mouseSensitivity>1.f||(defaultMode!=GameMode::Survival&&defaultMode!=GameMode::Creative))return false;
 for(std::size_t i=0;i<ACTION_COUNT;++i){auto a=static_cast<KeyAction>(i);int c=keybindings[i];if(!validCode(c)||(c==GLFW_KEY_ESCAPE&&a!=KeyAction::Pause)||!conflicts(a,c).empty())return false;}return true;
}
std::string SettingsState::bindingName(int code){
 if(code>=1000){const int button=code-1000;if(button==0)return "Mouse Left";if(button==1)return "Mouse Right";if(button==2)return "Mouse Middle";return "Mouse "+std::to_string(button+1);}
 if(code==32)return "Space";
 if(code>=33&&code<=96)return std::string(1,static_cast<char>(code));
 if(code>=GLFW_KEY_F1&&code<=GLFW_KEY_F25)return "F"+std::to_string(code-GLFW_KEY_F1+1);
 if(code>=GLFW_KEY_KP_0&&code<=GLFW_KEY_KP_9)return "Numpad "+std::to_string(code-GLFW_KEY_KP_0);
 switch(code){case GLFW_KEY_INSERT:return "Insert";case GLFW_KEY_DELETE:return "Delete";case GLFW_KEY_HOME:return "Home";case GLFW_KEY_END:return "End";case GLFW_KEY_PAGE_UP:return "Page Up";case GLFW_KEY_PAGE_DOWN:return "Page Down";case GLFW_KEY_CAPS_LOCK:return "Caps Lock";case GLFW_KEY_SCROLL_LOCK:return "Scroll Lock";case GLFW_KEY_NUM_LOCK:return "Num Lock";case GLFW_KEY_PRINT_SCREEN:return "Print Screen";case GLFW_KEY_PAUSE:return "Pause / Break";case GLFW_KEY_LEFT_SUPER:return "Left Super";case GLFW_KEY_RIGHT_SUPER:return "Right Super";case GLFW_KEY_MENU:return "Menu";case GLFW_KEY_WORLD_1:return "World 1";case GLFW_KEY_WORLD_2:return "World 2";case GLFW_KEY_KP_DECIMAL:return "Numpad Decimal";case GLFW_KEY_KP_DIVIDE:return "Numpad Divide";case GLFW_KEY_KP_MULTIPLY:return "Numpad Multiply";case GLFW_KEY_KP_SUBTRACT:return "Numpad Subtract";case GLFW_KEY_KP_ADD:return "Numpad Add";case GLFW_KEY_KP_ENTER:return "Numpad Enter";case GLFW_KEY_KP_EQUAL:return "Numpad Equal";case GLFW_KEY_ESCAPE:return "Escape";case GLFW_KEY_ENTER:return "Enter";case GLFW_KEY_TAB:return "Tab";case GLFW_KEY_BACKSPACE:return "Backspace";case GLFW_KEY_LEFT_CONTROL:return "Left Ctrl";case GLFW_KEY_RIGHT_CONTROL:return "Right Ctrl";case GLFW_KEY_LEFT_SHIFT:return "Left Shift";case GLFW_KEY_RIGHT_SHIFT:return "Right Shift";case GLFW_KEY_LEFT_ALT:return "Left Alt";case GLFW_KEY_RIGHT_ALT:return "Right Alt";case GLFW_KEY_UP:return "Up";case GLFW_KEY_DOWN:return "Down";case GLFW_KEY_LEFT:return "Left";case GLFW_KEY_RIGHT:return "Right";default:return "Key "+std::to_string(code);}
}
std::filesystem::path SettingsState::getSettingsPath(){return SaveLoad::getSavePath().parent_path()/"settings.cfg";}
bool SettingsState::load(SettingsState& out){return load(out,getSettingsPath());}
bool SettingsState::load(SettingsState& out,const std::filesystem::path& path){
 SettingsState parsed;std::ifstream in(path);std::string magic,name;int value;
 if(!std::getline(in,magic)||(magic!="MCSettings 1"&&magic!="MCSettings 2")){out.reset();return false;}
 if(!(in>>name>>parsed.viewDistance)||name!="view_distance"){out.reset();return false;}
 const bool legacy=magic=="MCSettings 1";
 if(!legacy){int mode;if(!(in>>name>>parsed.baseFov)||name!="base_fov"||!(in>>name>>parsed.mouseSensitivity)||name!="mouse_sensitivity"||!(in>>name>>parsed.vsync)||name!="vsync"||!(in>>name>>parsed.invertY)||name!="invert_y"||!(in>>name>>parsed.rawMouseInput)||name!="raw_mouse"||!(in>>name>>mode)||name!="default_mode"||mode<0||mode>1){out.reset();return false;}parsed.defaultMode=static_cast<GameMode>(mode);}
 const std::size_t count=legacy?19:ACTION_COUNT;
 for(std::size_t i=0;i<count;++i){if(!(in>>name>>value)||name!=names[i]||!validCode(value)||(value==GLFW_KEY_ESCAPE&&static_cast<KeyAction>(i)!=KeyAction::Pause)){out.reset();return false;}parsed.keybindings[i]=value;}
 if(in>>name){out.reset();return false;}
 if(legacy){parsed.mouseSensitivity=.10f;parsed.notice="Controls migrated; existing bindings preserved";
  for(std::size_t i=19;i<ACTION_COUNT;++i){const auto action=static_cast<KeyAction>(i);bool conflict=false;for(std::size_t j=0;j<i;++j)if(parsed.keybindings[j]==parsed.keybindings[i]&&contextsOverlap(action,static_cast<KeyAction>(j)))conflict=true;
   if(conflict){for(int candidate=32;candidate<=GLFW_KEY_LAST;++candidate)if(validCode(candidate)&&candidate!=256&&std::find(parsed.keybindings.begin(),parsed.keybindings.end(),candidate)==parsed.keybindings.end()){parsed.keybindings[i]=candidate;break;}}}
 }
 if(!parsed.valid()){out.reset();return false;}out=parsed;return true;
}
bool SettingsState::save(const SettingsState& s){return save(s,getSettingsPath());}
bool SettingsState::save(const SettingsState& s,const std::filesystem::path& path){
 if(!s.valid())return false;
 std::error_code ec;if(!path.parent_path().empty())std::filesystem::create_directories(path.parent_path(),ec);if(ec)return false;AtomicFile file(path);auto& out=file.stream;
 out<<"MCSettings 2\nview_distance "<<s.viewDistance<<"\nbase_fov "<<s.baseFov<<"\nmouse_sensitivity "<<s.mouseSensitivity<<"\nvsync "<<s.vsync<<"\ninvert_y "<<s.invertY<<"\nraw_mouse "<<s.rawMouseInput<<"\ndefault_mode "<<static_cast<int>(s.defaultMode)<<'\n';
 for(std::size_t i=0;i<ACTION_COUNT;++i)out<<names[i]<<' '<<s.keybindings[i]<<'\n';
 return file.commit();
}

const char* SettingsState::actionLabel(KeyAction action){
 static constexpr const char* labels[]={"Move Forward","Move Backward","Move Left","Move Right","Jump / Fly Up","Fly Down","Sprint","Inventory","Toggle Flight","Cycle Perspective","Hotbar 1","Hotbar 2","Hotbar 3","Hotbar 4","Hotbar 5","Hotbar 6","Hotbar 7","Hotbar 8","Hotbar 9","Pause","Attack / Break","Use / Place","Pick Block","Drop Item","Sneak","Open Chat","Toggle HUD","Zoom"};
 return static_cast<std::size_t>(action)<ACTION_COUNT?labels[static_cast<std::size_t>(action)]:"";
}
