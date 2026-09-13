#pragma once
#include <glm/glm.hpp>
class Player;
class World;
class PlayerCamera{
public:
  // Mirrors vanilla's F5 cycle: first person -> third person (behind) -> third person (front, "selfie").
  enum class POV:unsigned char{FIRST,THIRD_BACK,THIRD_FRONT};
  float baseFov=70.f,fov=70.f; POV pov=POV::FIRST;
  bool isThirdPerson()const{return pov!=POV::FIRST;}
  void cyclePOV(){pov=static_cast<POV>((static_cast<unsigned char>(pov)+1)%3);}
  void adjustFov(float delta);
  void updateFov(float dt,bool sprinting);
  glm::vec3 desiredPosition(const Player&)const;
  glm::vec3 position(const Player&,const World&)const;
  glm::vec3 direction(const Player&,const glm::vec3& cameraPosition)const;
  glm::mat4 view(const Player&,const glm::vec3& cameraPosition)const;
};

