#pragma once
#include <glm/glm.hpp>
class World;

class Player {
public:
  static constexpr float WIDTH=.6f,HEIGHT=1.8f,EYE_HEIGHT=1.62f;
  explicit Player(glm::vec3 spawn={500.f,8.f,500.f}):position(spawn){}
  glm::vec3 position,velocity{0}; float yaw=-90.f,pitch=-15.f; bool onGround=false,isFlying=false;
  glm::vec3 forward()const; glm::vec3 right()const; glm::vec3 eyePosition()const{return position+glm::vec3(0,EYE_HEIGHT,0);}
  void look(float dx,float dy); void update(float dt,const World&); void jump(); void toggleFly();
  bool overlapsBlock(const glm::ivec3&)const;
};

