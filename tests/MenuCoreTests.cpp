#include "core/SettingsState.h"
#include "core/InputResolver.h"
#include "core/WorldRepository.h"
#include "core/MenuController.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": "<<#x<<'\n';return 1;}}while(false)
std::string bytes(const std::filesystem::path& path){std::ifstream in(path,std::ios::binary);return {std::istreambuf_iterator<char>(in),{}};}
int main(){
 auto root=std::filesystem::temp_directory_path()/"minecraft-menu-core-tests";std::filesystem::create_directories(root);
 SettingsState settings;CHECK(settings.valid()&&settings.mouseSensitivity==.15f);
 CHECK(SettingsState::bindingName(1000)=="Mouse Left"&&SettingsState::bindingName(1003)=="Mouse 4");CHECK(!SettingsState::validCode(1008)&&!SettingsState::validCode(-1)&&!SettingsState::validCode(270));
 for(std::size_t i=0;i<SettingsState::ACTION_COUNT;++i)for(int code:{80,1007}){SettingsState s;auto a=static_cast<KeyAction>(i);CHECK(s.assign(a,code));InputResolver input;input.event(code,true);CHECK(input.justPressed(s,a)&&input.held(s,a,a==KeyAction::FlyDown));input.endFrame();CHECK(!input.justPressed(s,a));input.suppress();CHECK(!input.held(s,a,a==KeyAction::FlyDown));input.event(code,false);CHECK(input.justReleased(s,a));input.event(code,true);CHECK(input.held(s,a,a==KeyAction::FlyDown));}
 InputResolver quick;quick.event(1000,true);quick.event(1000,false);CHECK(quick.justPressed(settings,KeyAction::Attack)&&quick.justReleased(settings,KeyAction::Attack)&&!quick.held(settings,KeyAction::Attack));quick.endFrame();CHECK(!quick.justPressed(settings,KeyAction::Attack));
 InputResolver input;input.event(341,true);CHECK(input.held(settings,KeyAction::Sneak,false)&&!input.held(settings,KeyAction::FlyDown,false));CHECK(!input.held(settings,KeyAction::Sneak,true)&&input.held(settings,KeyAction::FlyDown,true));
 CHECK(!settings.assign(KeyAction::Attack,256));CHECK(!settings.assign(KeyAction::Attack,87));CHECK(settings.assign(KeyAction::Attack,87,true)&&settings.key(KeyAction::MoveForward)==1000);CHECK(!settings.assign(KeyAction::Attack,341,true));settings.baseFov=81;settings.resetBindings();CHECK(settings.valid()&&settings.baseFov==81);
 CHECK(SettingsState::save(settings,root/"settings.cfg"));SettingsState loaded;CHECK(SettingsState::load(loaded,root/"settings.cfg")&&loaded.baseFov==81);
 {std::ofstream old(root/"old.cfg");old<<"MCSettings 1\nview_distance\n6\n";for(int i=0;i<19;++i)old<<SettingsState::actionName(static_cast<KeyAction>(i))<<' '<<(i==0?81:SettingsState::defaultBinding(static_cast<KeyAction>(i)))<<'\n';}
 CHECK(SettingsState::load(loaded,root/"old.cfg")&&loaded.mouseSensitivity==.10f&&loaded.key(KeyAction::MoveForward)==81&&loaded.key(KeyAction::DropItem)!=81&&!loaded.notice.empty()&&loaded.valid());
 WorldRepository repo(root);WorldInfo one,two;std::string error;
 CHECK(!repo.create("   ",0,GameMode::Survival,one,error));CHECK(!repo.create(std::string(49,'a'),0,GameMode::Survival,one,error));CHECK(repo.create(" Same ",0,GameMode::Survival,one,error));CHECK(repo.create("Same",4294967295u,GameMode::Creative,two,error)&&one.id!=two.id&&one.name==two.name);CHECK(repo.list().size()==2);
 SaveData data;CHECK(repo.load(two.id,data,error)&&data.seed==4294967295u&&data.mode==GameMode::Creative);data.playerX=25;data.playerY=30;data.playerZ=40;data.yaw=120;data.pitch=15;data.timeOfDay=.8f;CHECK(repo.save(two,data,error));SaveData restored;CHECK(repo.load(two.id,restored,error)&&restored.playerX==25&&restored.yaw==120&&restored.timeOfDay==.8f);
 auto save=root/"worlds"/two.id/"save.dat";auto before=bytes(save);auto invalid=data;invalid.timeOfDay=std::numeric_limits<float>::quiet_NaN();CHECK(!repo.save(two,invalid,error)&&bytes(save)==before);
 std::filesystem::create_directory(save.string()+".tmp");CHECK(!repo.save(two,data,error)&&bytes(save)==before);
 CHECK(!repo.remove("../",error)&&!repo.remove("Same",error)&&repo.list().size()==2);CHECK(repo.remove(one.id,error)&&repo.list().size()==1);
 {std::ofstream corrupt(root/"worlds"/two.id/"world.meta");corrupt<<"invalid";}CHECK(!repo.list()[0].error.empty());
 CHECK(SaveLoad::save(SaveData{},root/"save.dat"));auto legacy=bytes(root/"save.dat");CHECK(repo.importLegacy(error));auto count=repo.list().size();CHECK(repo.importLegacy(error)&&repo.list().size()==count&&bytes(root/"save.dat")==legacy);bool imported=false;for(auto& w:repo.list())if(w.imported){imported=true;CHECK(bytes(root/"worlds"/w.id/"save.dat")==legacy);}CHECK(imported);
 TextField field;field.insert('a',3);field.insert('b',3);field.edit(263);field.insert('c',3);CHECK(field.text=="acb");field.insert('d',3);CHECK(field.text=="acb");field.edit(259);CHECK(field.text=="ab");
 MenuView view;view.widgets={{1,"A",0,0,100,50},{2,"B",0,60,100,50,false},{3,"C",0,120,100,50}};MenuController menu;menu.navigate(view,1);CHECK(menu.focus==2);menu.navigate(view,1);CHECK(menu.focus==0);menu.navigate(view,-1);CHECK(menu.focus==2&&menu.hit(view,50,70)==-1&&menu.hit(view,50,140)==2);
 std::filesystem::remove_all(root);return 0;
}
