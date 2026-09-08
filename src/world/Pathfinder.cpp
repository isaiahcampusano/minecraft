#include "Pathfinder.h"
#include "World.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <unordered_map>

namespace{
struct Key{int x,y,z;bool operator==(const Key& other)const{return x==other.x&&y==other.y&&z==other.z;}};
struct KeyHash{std::size_t operator()(const Key& key)const{std::size_t h=static_cast<unsigned>(key.x);h=(h*16777619u)^static_cast<unsigned>(key.y);return(h*16777619u)^static_cast<unsigned>(key.z);}};
struct OpenNode{Key key;float score;bool operator<(const OpenNode& other)const{return score>other.score;}};
Key key(const glm::ivec3& value){return{value.x,value.y,value.z};}
glm::ivec3 position(const Key& value){return{value.x,value.y,value.z};}
float heuristic(const glm::ivec3& a,const glm::ivec3& b){return static_cast<float>(std::abs(a.x-b.x)+std::abs(a.z-b.z))+std::abs(a.y-b.y)*1.5f;}
}

bool Pathfinder::isWalkable(const World& world,const glm::ivec3& feet){return isSolid(world.getBlock(feet.x,feet.y-1,feet.z))&&!isSolid(world.getBlock(feet.x,feet.y,feet.z))&&!isSolid(world.getBlock(feet.x,feet.y+1,feet.z));}

std::vector<glm::ivec3> Pathfinder::findPath(const World& world,const glm::ivec3& start,const glm::ivec3& target,int maxRange,int maxVisited){
  if(maxRange<=0||maxVisited<=0||!isWalkable(world,start)||!isWalkable(world,target))return{};
  std::priority_queue<OpenNode> open;std::unordered_map<Key,float,KeyHash> cost;std::unordered_map<Key,Key,KeyHash> parent;
  const Key startKey=key(start),targetKey=key(target);cost[startKey]=0;open.push({startKey,heuristic(start,target)});int visited=0;
  while(!open.empty()&&visited<maxVisited){const Key currentKey=open.top().key;open.pop();const glm::ivec3 current=position(currentKey);++visited;if(currentKey==targetKey){std::vector<glm::ivec3> path;Key step=currentKey;while(!(step==startKey)){path.push_back(position(step));step=parent.at(step);}std::reverse(path.begin(),path.end());return path;}
    static constexpr int directions[4][2]={{1,0},{-1,0},{0,1},{0,-1}};
    for(const auto& direction:directions){glm::ivec3 next{current.x+direction[0],current.y,current.z+direction[1]};if(!isWalkable(world,next)){if(isWalkable(world,next+glm::ivec3(0,1,0)))++next.y;else if(isWalkable(world,next-glm::ivec3(0,1,0)))--next.y;else continue;}if(std::abs(next.x-start.x)+std::abs(next.z-start.z)>maxRange||std::abs(next.y-start.y)>maxRange)continue;const float nextCost=cost[currentKey]+1.f+(next.y!=current.y?.5f:0.f);const Key nextKey=key(next);const auto existing=cost.find(nextKey);if(existing!=cost.end()&&existing->second<=nextCost)continue;cost[nextKey]=nextCost;parent[nextKey]=currentKey;open.push({nextKey,nextCost+heuristic(next,target)});}
  }
  return{};
}
