#include "PlayerCamera.h"
#include "Player.h"
#include "../utils/Raycaster.h"
#include "../world/World.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
void PlayerCamera::adjustFov(float delta){baseFov=std::clamp(baseFov+delta,30.f,90.f);}
void PlayerCamera::updateFov(float dt,bool sprinting){const float target=std::min(baseFov+(sprinting?10.f:0.f),100.f),factor=1.f-std::exp(-8.f*std::max(0.f,dt));fov+=(target-fov)*factor;}
glm::vec3 PlayerCamera::desiredPosition(const Player&p)const{if(!thirdPerson)return p.eyePosition();const glm::vec3 pivot=p.position+glm::vec3(0,1.15f,0);return pivot-p.forward()*5.f+p.right()*1.35f+glm::vec3(0,1.35f,0);}
glm::vec3 PlayerCamera::position(const Player&p,const World&w)const{if(!thirdPerson)return p.eyePosition();const glm::vec3 pivot=p.position+glm::vec3(0,1.15f,0),desired=desiredPosition(p),offset=desired-pivot;const float desiredDistance=glm::length(offset);if(desiredDistance<=0.f)return pivot;const glm::vec3 rayDirection=offset/desiredDistance;const RayHit hit=Raycaster::cast(w,pivot,rayDirection,desiredDistance);if(!hit.hit)return desired;constexpr float surfaceBuffer=.2f;return pivot+rayDirection*std::max(0.f,hit.distance-surfaceBuffer);}
glm::vec3 PlayerCamera::direction(const Player&p,const glm::vec3&cameraPosition)const{if(!thirdPerson)return p.forward();const glm::vec3 target=p.position+glm::vec3(0,1.25f,0);return glm::normalize(target-cameraPosition);}
glm::mat4 PlayerCamera::view(const Player&p,const glm::vec3&cameraPosition)const{return glm::lookAt(cameraPosition,cameraPosition+direction(p,cameraPosition),{0,1,0});}
