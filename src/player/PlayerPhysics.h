#pragma once
#include <glm/glm.hpp>
class Player;class World;
class PlayerPhysics{public:static void update(Player&,float,const World&);static bool collides(const Player&,const glm::vec3&,const World&);};

