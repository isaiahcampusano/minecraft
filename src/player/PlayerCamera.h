#pragma once
#include <glm/glm.hpp>
class Player;
class PlayerCamera{
public: float fov=70.f; bool thirdPerson=false;
  void togglePOV(){thirdPerson=!thirdPerson;} glm::vec3 position(const Player&)const; glm::vec3 direction(const Player&)const; glm::mat4 view(const Player&)const;
};

