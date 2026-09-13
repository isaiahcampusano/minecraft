#pragma once
#include <algorithm>
#include <string>
#include <vector>

// Coordinates use a 1280 x 720 canvas for both drawing and input.
struct MenuWidget {
  int id; std::string label; float x,y,w,h; bool enabled=true; float value=-1;
  bool contains(float px,float py)const{return px>=x&&px<x+w&&py>=y&&py<y+h;}
};
struct MenuView {std::string title,notice;std::vector<MenuWidget> widgets;};
class MenuController {
public:
  int focus=0,hover=-1,pressed=-1,scroll=0;
  void reset(){focus=0;hover=pressed=-1;scroll=0;}
  int hit(const MenuView& view,float x,float y)const{for(std::size_t i=0;i<view.widgets.size();++i)if(view.widgets[i].enabled&&view.widgets[i].contains(x,y))return static_cast<int>(i);return -1;}
  void navigate(const MenuView& view,int direction){if(view.widgets.empty())return;for(std::size_t n=0;n<view.widgets.size();++n){focus=(focus+direction+static_cast<int>(view.widgets.size()))%static_cast<int>(view.widgets.size());if(view.widgets[focus].enabled)return;}}
};
struct TextField {
  std::string text;std::size_t caret=0;
  void insert(unsigned code,std::size_t limit){if(code>=32&&code<=126&&text.size()<limit){text.insert(caret,1,static_cast<char>(code));++caret;}}
  void edit(int key){if(key==259&&caret){text.erase(--caret,1);}else if(key==261&&caret<text.size())text.erase(caret,1);else if(key==263&&caret)--caret;else if(key==262&&caret<text.size())++caret;else if(key==268)caret=0;else if(key==269)caret=text.size();}
  void set(std::string value){text=std::move(value);caret=text.size();}
  std::string display()const{auto result=text;result.insert(caret,"|");return result;}
};
