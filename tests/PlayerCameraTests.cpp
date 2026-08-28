#include "player/PlayerCamera.h"
#include "player/Player.h"
#include "world/World.h"
#include <cmath>
#include <iostream>

namespace{int fail(const char*message){std::cerr<<message<<'\n';return 1;}bool near(const glm::vec3&a,const glm::vec3&b,float epsilon=.06f){return glm::length(a-b)<epsilon;}}

int main(){
  World world;Player player({500.f,8.f,500.f});world.loadChunk(31,31);PlayerCamera camera;
  if(!near(camera.position(player,world),player.eyePosition()))return fail("First-person camera position changed");
  camera.thirdPerson=true;const glm::vec3 desired=camera.desiredPosition(player);if(!near(camera.position(player,world),desired))return fail("Unobstructed third-person camera did not reach its desired offset");
  const glm::vec3 pivot=player.position+glm::vec3(0,1.15f,0),ray=glm::normalize(desired-pivot);const glm::ivec3 obstruction=glm::floor(pivot+ray*2.5f);world.setBlock(obstruction.x,obstruction.y,obstruction.z,BlockType::STONE);
  const glm::vec3 clamped=camera.position(player,world);if(glm::length(clamped-pivot)>=glm::length(desired-pivot)-.5f)return fail("Third-person camera was not pulled in by solid geometry");if(isSolid(world.getBlock(static_cast<int>(std::floor(clamped.x)),static_cast<int>(std::floor(clamped.y)),static_cast<int>(std::floor(clamped.z)))))return fail("Clamped camera remained inside solid geometry");
  return 0;
}
