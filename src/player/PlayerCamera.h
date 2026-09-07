#pragma once
#include <glm/glm.hpp>
class Player;
class World;
class PlayerCamera{
public: float baseFov=70.f,fov=70.f; bool thirdPerson=false;
  void togglePOV(){thirdPerson=!thirdPerson;}
  void adjustFov(float delta);
  void updateFov(float dt,bool sprinting);
  glm::vec3 desiredPosition(const Player&)const;
  glm::vec3 position(const Player&,const World&)const;
  glm::vec3 direction(const Player&,const glm::vec3& cameraPosition)const;
  glm::mat4 view(const Player&,const glm::vec3& cameraPosition)const;
};

