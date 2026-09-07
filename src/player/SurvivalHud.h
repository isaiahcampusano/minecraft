#pragma once
#include <algorithm>

inline int survivalIconValue(int level,int index){return std::clamp(level-index*2,0,2);}
