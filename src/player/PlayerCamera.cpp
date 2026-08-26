#include "PlayerCamera.h"
#include "Player.h"
#include <glm/gtc/matrix_transform.hpp>
glm::vec3 PlayerCamera::position(const Player&p)const{if(!thirdPerson)return p.eyePosition();glm::vec3 target=p.position+glm::vec3(0,1.25f,0);return target-p.forward()*5.f+glm::vec3(0,1.2f,0);}
glm::vec3 PlayerCamera::direction(const Player&p)const{if(!thirdPerson)return p.forward();glm::vec3 target=p.position+glm::vec3(0,1.25f,0);return glm::normalize(target-position(p));}
glm::mat4 PlayerCamera::view(const Player&p)const{return glm::lookAt(position(p),position(p)+direction(p),{0,1,0});}

