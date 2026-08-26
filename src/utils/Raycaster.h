#pragma once
#include <glm/glm.hpp>
class World;
struct RayHit{bool hit=false;glm::ivec3 block{0};glm::ivec3 adjacent{0};};
class Raycaster{public:static RayHit cast(const World&,glm::vec3 origin,glm::vec3 direction,float maxDistance=6.f);};

