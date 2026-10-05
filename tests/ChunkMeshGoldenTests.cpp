#include "world/Chunk.h"
#include <cstddef>
#include <cstdint>
#include <iostream>

namespace {
struct Fingerprint {
  std::size_t vertices;
  std::size_t indices;
  std::uint64_t vertexHash;
  std::uint64_t indexHash;
};

std::uint64_t checksum(const void* data,std::size_t size){
  const auto* bytes=static_cast<const unsigned char*>(data);
  std::uint64_t hash=14695981039346656037ull;
  for(std::size_t i=0;i<size;++i){hash^=bytes[i];hash*=1099511628211ull;}
  return hash;
}

auto worldBlock(Chunk& chunk){
  const auto position=chunk.position();
  return [&chunk,position](int x,int y,int z){
    return chunk.getBlock(x-position.x*Chunk::SIZE_X,y,z-position.y*Chunk::SIZE_Z);
  };
}

Fingerprint meshFingerprint(Chunk& chunk){
  const auto mesh=chunk.buildMeshData(worldBlock(chunk));
  return {mesh.vertices.size(),mesh.indices.size(),
          checksum(mesh.vertices.data(),mesh.vertices.size()*sizeof(Vertex)),
          checksum(mesh.indices.data(),mesh.indices.size()*sizeof(unsigned))};
}

void makeSuperflat(Chunk& chunk){
  for(int z=0;z<Chunk::SIZE_Z;++z)for(int x=0;x<Chunk::SIZE_X;++x){
    chunk.setBlock(x,0,z,BlockType::BEDROCK);
    for(int y=1;y<=3;++y)chunk.setBlock(x,y,z,BlockType::DIRT);
    chunk.setBlock(x,4,z,BlockType::GRASS);
  }
}

}

int main(){
  static_assert(sizeof(Vertex)==sizeof(float)*6,"vertex buffer layout must remain six floats");
  static_assert(sizeof(unsigned)==sizeof(std::uint32_t),"index buffer layout must remain 32-bit");
  int result=0;
  {
    Chunk flat({0,0});makeSuperflat(flat);
    // Golden hashes recorded with 18 block types (COAL_ORE/IRON_ORE extend the
    // atlas vs the original 16-type golden). Geometry/topology are unchanged:
    // vertex/index counts and the index hashes match the pre-ore golden.
    const Fingerprint flatExpected{3328,4992,0x9c6821bf96f99fd1ull,0x05ecadf1f8948b25ull};
    const auto flatActual=meshFingerprint(flat);
    if(flatActual.vertices!=flatExpected.vertices||flatActual.indices!=flatExpected.indices||
       flatActual.vertexHash!=flatExpected.vertexHash||flatActual.indexHash!=flatExpected.indexHash){
      std::cerr<<"superflat mesh output changed\n";result=1;
    }
    Chunk edited({0,0});makeSuperflat(edited);
    edited.setBlock(0,5,0,BlockType::OAK_LOG);
    edited.setBlock(0,6,0,BlockType::LEAVES);
    edited.setBlock(15,5,15,BlockType::STONE);
    edited.setBlock(15,6,15,BlockType::GLASS);
    const Fingerprint editedExpected{3392,5088,0xe1a8f0c85ed006edull,0x2aa844935d733725ull};
    const auto editedActual=meshFingerprint(edited);
    if(editedActual.vertices!=editedExpected.vertices||editedActual.indices!=editedExpected.indices||
       editedActual.vertexHash!=editedExpected.vertexHash||editedActual.indexHash!=editedExpected.indexHash){
      std::cerr<<"edge-edited mesh output changed\n";result=1;
    }
  }
  return result;
}
