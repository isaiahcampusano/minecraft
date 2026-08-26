#include "PlayerPhysics.h"
#include "Player.h"
#include "../world/World.h"
#include <algorithm>
#include <cmath>
bool PlayerPhysics::collides(const Player&,const glm::vec3&p,const World&w){constexpr float e=.0001f;glm::vec3 lo=p+glm::vec3(-Player::WIDTH*.5f,0,-Player::WIDTH*.5f),hi=p+glm::vec3(Player::WIDTH*.5f,Player::HEIGHT,Player::WIDTH*.5f);
  for(int y=static_cast<int>(std::floor(lo.y));y<=static_cast<int>(std::floor(hi.y-e));++y)
    for(int z=static_cast<int>(std::floor(lo.z));z<=static_cast<int>(std::floor(hi.z-e));++z)
      for(int x=static_cast<int>(std::floor(lo.x));x<=static_cast<int>(std::floor(hi.x-e));++x)
        if(isSolid(w.getBlock(x,y,z)))return true;
  return false;
}
void PlayerPhysics::update(Player&p,float dt,const World&w){if(!p.isFlying)p.velocity.y=std::max(p.velocity.y-25.f*dt,-50.f);
  for(int axis=0;axis<3;++axis){glm::vec3 start=p.position,candidate=start;candidate[axis]+=p.velocity[axis]*dt;if(!collides(p,candidate,w))p.position=candidate;else{float safe=start[axis],blocked=candidate[axis];for(int i=0;i<12;++i){float mid=(safe+blocked)*.5f;glm::vec3 probe=start;probe[axis]=mid;if(collides(p,probe,w))blocked=mid;else safe=mid;}p.position[axis]=safe;p.velocity[axis]=0;}}
  p.position.x=std::clamp(p.position.x,Player::WIDTH*.5f,1000.f-Player::WIDTH*.5f);p.position.z=std::clamp(p.position.z,Player::WIDTH*.5f,1000.f-Player::WIDTH*.5f);
  p.onGround=!p.isFlying&&collides(p,p.position-glm::vec3(0,.03f,0),w);
}
