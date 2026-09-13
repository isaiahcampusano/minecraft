#pragma once
#include <array>

namespace FurnaceLayout {
struct Rect { float x,y,w,h; bool contains(float px,float py)const{return px>=x&&px<x+w&&py>=y&&py<y+h;} };
inline std::array<Rect,3> slots(int width){const float left=(width-464.f)*.5f;return {{{left+110.f,350.f,48.f,48.f},{left+110.f,252.f,48.f,48.f},{left+330.f,304.f,48.f,48.f}}};}
inline Rect arrow(int width){return{(width-464.f)*.5f+200.f,319.f,85.f,18.f};}
inline Rect flame(int width){return{(width-464.f)*.5f+124.f,312.f,20.f,26.f};}
inline int hitTest(float x,float y,int width){const auto rects=slots(width);for(int i=0;i<3;++i)if(rects[i].contains(x,y))return i;return -1;}
}
