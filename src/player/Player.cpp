#include "Player.h"
#include "PlayerPhysics.h"
#include <glm/common.hpp>
#include <cmath>
glm::vec3 Player::forward()const{glm::vec3 f{std::cos(glm::radians(yaw))*std::cos(glm::radians(pitch)),std::sin(glm::radians(pitch)),std::sin(glm::radians(yaw))*std::cos(glm::radians(pitch))};return glm::normalize(f);}
glm::vec3 Player::right()const{return glm::normalize(glm::cross(forward(),glm::vec3{0,1,0}));}
void Player::look(float dx,float dy){yaw+=dx*.1f;pitch=glm::clamp(pitch+dy*.1f,-89.f,89.f);}
void Player::update(float dt,const World&w){PlayerPhysics::update(*this,dt,w);}
bool Player::jump(bool sprinting){if(!onGround||isFlying)return false;velocity.y=8.5f;onGround=false;survival.addExhaustion(sprinting?.2f:.05f);return true;}
void Player::toggleFly(){isFlying=!isFlying;velocity.y=0;if(isFlying)survival.resetFallDistance();}
void Player::respawn(const glm::vec3& spawn){position=spawn;velocity={0,0,0};onGround=false;isFlying=false;survival.reset();clearMiningTarget();resetMiningCooldown();}
bool Player::overlapsBlock(const glm::ivec3&b)const{glm::vec3 p0=position+glm::vec3(-WIDTH*.5f,0,-WIDTH*.5f),p1=position+glm::vec3(WIDTH*.5f,HEIGHT,WIDTH*.5f),b0=b,b1=glm::vec3(b)+glm::vec3(1);return p0.x<b1.x&&p1.x>b0.x&&p0.y<b1.y&&p1.y>b0.y&&p0.z<b1.z&&p1.z>b0.z;}

