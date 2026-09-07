#include "world/LootTable.h"
#include <iostream>
int main(){LootTable loot(1234),again(1234);int apples=0;for(int i=0;i<20000;++i){const bool a=loot.dropsApple(BlockType::LEAVES),b=again.dropsApple(BlockType::LEAVES);if(a!=b){std::cerr<<"seeded loot was not deterministic\n";return 1;}apples+=a?1:0;}if(apples<850||apples>1150){std::cerr<<"leaf drop rate was not near 5%: "<<apples<<'\n';return 1;}for(int i=0;i<100;++i)if(loot.dropsApple(BlockType::DIRT)){std::cerr<<"non-leaf dropped an apple\n";return 1;}return 0;}
