#include "ChunkRenderer.h"
#include "../world/World.h"
#include <set>

void ChunkRenderer::render(World& world,const Frustum& frustum){
  std::set<ChunkKey> loaded;
  for(const auto& position:world.loadedChunkPositions())loaded.emplace(position.x,position.y);
  for(auto it=m_meshes.begin();it!=m_meshes.end();)
    if(!loaded.count(it->first))it=m_meshes.erase(it);
    else ++it;

  for(auto& upload:world.takeChunkMeshUploads()){
    const ChunkKey key{upload.position.x,upload.position.y};
    auto& e=m_meshes[key];
    e.mesh.upload(upload.mesh);
    e.mn=glm::vec3(upload.position.x*16.f,0.f,upload.position.y*16.f);
    e.mx=glm::vec3(upload.position.x*16.f+16.f,float(upload.maxY+1),upload.position.y*16.f+16.f);
  }
  for(const auto& kv:m_meshes)
    if(frustum.intersects(kv.second.mn,kv.second.mx))kv.second.mesh.draw();
}
