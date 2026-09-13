#include "world/World.h"
#include "player/Player.h"
#include "player/PlayerPhysics.h"
#include <iostream>
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": "<<#x<<'\n';return 1;}}while(false)
int main(){World a(17),b(17),c(18);for(auto* world:{&a,&b,&c}){world->loadChunk(1,1);world->loadChunk(2,1);}bool differs=false;for(int x=16;x<48;++x)for(int z=16;z<32;++z)for(int y=0;y<16;++y){CHECK(a.getBlock(x,y,z)==b.getBlock(x,y,z));if(a.getBlock(x,y,z)!=c.getBlock(x,y,z))differs=true;}CHECK(differs);a.unloadChunk(1,1);a.loadChunk(1,1);for(int x=16;x<32;++x)for(int z=16;z<32;++z)for(int y=0;y<16;++y)CHECK(a.getBlock(x,y,z)==b.getBlock(x,y,z));
 World ledge;ledge.setBlock(20,20,20,BlockType::STONE);Player player({20.5f,21,20.5f});player.onGround=true;player.isSneaking=true;for(int i=0;i<40;++i){player.velocity={2,0,2};PlayerPhysics::update(player,.05f,ledge);}CHECK(player.position.x<21.3f&&player.position.z<21.3f&&player.onGround);player.velocity={2,9,2};PlayerPhysics::update(player,.05f,ledge);CHECK(player.position.y>21);return 0;}
