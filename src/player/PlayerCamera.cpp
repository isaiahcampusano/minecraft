#include "PlayerCamera.h"
#include "Player.h"
#include "../utils/Raycaster.h"
#include "../world/World.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
void PlayerCamera::adjustFov(float delta){baseFov=std::clamp(baseFov+delta,30.f,90.f);}
void PlayerCamera::updateFov(float dt,bool sprinting){const float target=std::min(baseFov+(sprinting?10.f:0.f),100.f),factor=1.f-std::exp(-8.f*std::max(0.f,dt));fov+=(target-fov)*factor;}
glm::vec3 PlayerCamera::desiredPosition(const Player&p)const{
  if(pov==POV::FIRST)return p.eyePosition();
  const glm::vec3 pivot=p.position+glm::vec3(0,1.15f,0);
  if(pov==POV::THIRD_BACK)return pivot-p.forward()*5.f+p.right()*1.35f+glm::vec3(0,1.35f,0);
  // THIRD_FRONT: centered out in front of the player, at the same height as the back-view rig.
  // No sideways offset here (unlike THIRD_BACK) so the player is framed dead-center, matching
  // vanilla's centered "selfie" framing rather than the over-the-shoulder back-view angle.
  return pivot+p.forward()*4.f+glm::vec3(0,1.35f,0);
}
glm::vec3 PlayerCamera::position(const Player&p,const World&w)const{if(pov==POV::FIRST)return p.eyePosition();const glm::vec3 pivot=p.position+glm::vec3(0,1.15f,0),desired=desiredPosition(p),offset=desired-pivot;const float desiredDistance=glm::length(offset);if(desiredDistance<=0.f)return pivot;const glm::vec3 rayDirection=offset/desiredDistance;const RayHit hit=Raycaster::cast(w,pivot,rayDirection,desiredDistance);if(!hit.hit)return desired;constexpr float surfaceBuffer=.2f;return pivot+rayDirection*std::max(0.f,hit.distance-surfaceBuffer);}
// Used only to build the on-screen view matrix. Both third-person modes aim back at the player's
// chest, which for THIRD_BACK looks like "look where I'm walking" and for THIRD_FRONT looks like
// "look at my own face" -- exactly what changes is where position() put the camera, not this math.
glm::vec3 PlayerCamera::direction(const Player&p,const glm::vec3&cameraPosition)const{if(pov==POV::FIRST)return p.forward();const glm::vec3 target=p.position+glm::vec3(0,1.25f,0);return glm::normalize(target-cameraPosition);}
glm::mat4 PlayerCamera::view(const Player&p,const glm::vec3&cameraPosition)const{return glm::lookAt(cameraPosition,cameraPosition+direction(p,cameraPosition),{0,1,0});}
