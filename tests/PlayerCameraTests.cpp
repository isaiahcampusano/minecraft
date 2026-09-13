#include "player/PlayerCamera.h"
#include "player/Player.h"
#include "world/World.h"
#include <cmath>
#include <iostream>

namespace{int fail(const char*message){std::cerr<<message<<'\n';return 1;}bool near(const glm::vec3&a,const glm::vec3&b,float epsilon=.06f){return glm::length(a-b)<epsilon;}}

int main(){
  World world;Player player({500.f,8.f,500.f});world.loadChunk(31,31);PlayerCamera camera;
  if(camera.pov!=PlayerCamera::POV::FIRST||camera.isThirdPerson())return fail("Camera did not default to first person");
  if(!near(camera.position(player,world),player.eyePosition()))return fail("First-person camera position changed");
  camera.pov=PlayerCamera::POV::THIRD_BACK;if(!camera.isThirdPerson())return fail("THIRD_BACK did not report as third person");const glm::vec3 desired=camera.desiredPosition(player);if(!near(camera.position(player,world),desired))return fail("Unobstructed third-person camera did not reach its desired offset");
  const glm::vec3 pivot=player.position+glm::vec3(0,1.15f,0),ray=glm::normalize(desired-pivot);const glm::ivec3 obstruction=glm::floor(pivot+ray*2.5f);world.setBlock(obstruction.x,obstruction.y,obstruction.z,BlockType::STONE);
  const glm::vec3 clamped=camera.position(player,world);if(glm::length(clamped-pivot)>=glm::length(desired-pivot)-.5f)return fail("Third-person camera was not pulled in by solid geometry");if(isSolid(world.getBlock(static_cast<int>(std::floor(clamped.x)),static_cast<int>(std::floor(clamped.y)),static_cast<int>(std::floor(clamped.z)))))return fail("Clamped camera remained inside solid geometry");
  world.setBlock(obstruction.x,obstruction.y,obstruction.z,BlockType::AIR);
  camera.pov=PlayerCamera::POV::THIRD_FRONT;if(!camera.isThirdPerson())return fail("THIRD_FRONT did not report as third person");
  const glm::vec3 frontDesired=camera.desiredPosition(player);if(glm::dot(frontDesired-pivot,player.forward())<=0.f)return fail("THIRD_FRONT camera was not placed in front of the player");
  if(near(frontDesired,desired))return fail("THIRD_FRONT used the same offset as THIRD_BACK");
  const glm::vec3 frontDir=camera.direction(player,camera.position(player,world));if(glm::dot(frontDir,player.forward())>=0.f)return fail("THIRD_FRONT camera did not look back toward the player");
  camera.pov=PlayerCamera::POV::FIRST;
  camera.cyclePOV();if(camera.pov!=PlayerCamera::POV::THIRD_BACK)return fail("cyclePOV did not advance FIRST -> THIRD_BACK");
  camera.cyclePOV();if(camera.pov!=PlayerCamera::POV::THIRD_FRONT)return fail("cyclePOV did not advance THIRD_BACK -> THIRD_FRONT");
  camera.cyclePOV();if(camera.pov!=PlayerCamera::POV::FIRST)return fail("cyclePOV did not wrap THIRD_FRONT -> FIRST");
  camera.adjustFov(100.f);if(camera.baseFov!=90.f)return fail("base FOV upper clamp failed");camera.updateFov(1.f,true);if(camera.fov<99.9f||camera.fov>100.f)return fail("sprint FOV did not approach capped target");camera.adjustFov(-100.f);if(camera.baseFov!=30.f)return fail("base FOV lower clamp failed");const float widened=camera.fov;camera.updateFov(.1f,false);if(camera.fov>=widened||camera.fov<=30.f)return fail("FOV recovery was not smooth");
  return 0;
}
