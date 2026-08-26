#include "Raycaster.h"
#include "../world/World.h"
#include <cmath>
RayHit Raycaster::cast(const World&w,glm::vec3 o,glm::vec3 d,float maxD){glm::ivec3 previous=glm::floor(o);for(float t=0;t<=maxD;t+=.05f){glm::ivec3 cell=glm::floor(o+d*t);if(isSolid(w.getBlock(cell.x,cell.y,cell.z)))return{true,cell,previous};previous=cell;}return{};}
