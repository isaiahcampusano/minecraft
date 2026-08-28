#include "ParticleSystem.h"
#include "Texture.h"
#include "../world/BlockRegistry.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {
const char* PARTICLE_VS=R"(#version 330 core
layout(location=0)in vec3 p;layout(location=1)in vec2 uv;layout(location=2)in float shade;
out vec2 vUV;out float vShade;uniform mat4 projection,view,model;uniform vec4 uvRegion;
void main(){vUV=mix(uvRegion.xy,uvRegion.zw,uv);vShade=shade;gl_Position=projection*view*model*vec4(p,1.0);})";
const char* PARTICLE_FS=R"(#version 330 core
in vec2 vUV;in float vShade;out vec4 color;uniform sampler2D atlas;uniform float alpha,daylight;
void main(){vec4 base=texture(atlas,vUV);color=vec4(base.rgb*vShade*mix(0.2,1.0,daylight),base.a*alpha);})";

struct ParticleVertex{float x,y,z,u,v,shade;};
}

ParticleSystem::ParticleSystem():m_shader(PARTICLE_VS,PARTICLE_FS){
  static constexpr float corners[8][3]={{-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,-.5f,.5f},{-.5f,-.5f,.5f},{-.5f,.5f,-.5f},{.5f,.5f,-.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f}};
  static constexpr int faces[6][4]={{1,5,6,2},{0,3,7,4},{4,7,6,5},{0,1,2,3},{2,6,7,3},{0,4,5,1}};
  static constexpr float shades[6]={.82f,.72f,1.f,.55f,.9f,.68f};
  static constexpr float uvs[4][2]={{0,0},{1,0},{1,1},{0,1}};
  std::array<ParticleVertex,36> vertices{};std::size_t out=0;
  for(int face=0;face<6;++face)for(int index:{0,1,2,0,2,3}){const int corner=faces[face][index];vertices[out++]={corners[corner][0],corners[corner][1],corners[corner][2],uvs[index][0],uvs[index][1],shades[face]};}
  glGenVertexArrays(1,&m_vao);glGenBuffers(1,&m_vbo);glBindVertexArray(m_vao);glBindBuffer(GL_ARRAY_BUFFER,m_vbo);glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices.data(),GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(ParticleVertex),reinterpret_cast<void*>(offsetof(ParticleVertex,x)));glEnableVertexAttribArray(0);
  glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(ParticleVertex),reinterpret_cast<void*>(offsetof(ParticleVertex,u)));glEnableVertexAttribArray(1);
  glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,sizeof(ParticleVertex),reinterpret_cast<void*>(offsetof(ParticleVertex,shade)));glEnableVertexAttribArray(2);glBindVertexArray(0);
}
ParticleSystem::~ParticleSystem(){if(m_vbo)glDeleteBuffers(1,&m_vbo);if(m_vao)glDeleteVertexArrays(1,&m_vao);}
float ParticleSystem::random01(){m_randomState^=m_randomState<<13;m_randomState^=m_randomState>>17;m_randomState^=m_randomState<<5;return(m_randomState&0x00ffffffu)/static_cast<float>(0x01000000u);}
ParticleSystem::Particle& ParticleSystem::acquire(){for(auto& particle:m_particles)if(!particle.active)return particle;return *std::max_element(m_particles.begin(),m_particles.end(),[](const Particle&a,const Particle&b){return a.age/a.lifetime<b.age/b.lifetime;});}
void ParticleSystem::spawnBlockBreak(const glm::ivec3& block,BlockType type){
  constexpr int debrisCount=12;const float tileWidth=1.f/static_cast<float>(BLOCK_TYPE_COUNT);const float tile=static_cast<float>(type);
  for(int i=0;i<debrisCount;++i){auto& particle=acquire();glm::vec3 direction{random01()*2.f-1.f,.35f+random01(),random01()*2.f-1.f};if(glm::dot(direction,direction)<.01f)direction={0,1,0};direction=glm::normalize(direction);
    const int sampleX=static_cast<int>(random01()*4.f),sampleY=static_cast<int>(random01()*4.f);const float u0=(tile+sampleX*.25f+.015f)*tileWidth,u1=(tile+(sampleX+1)*.25f-.015f)*tileWidth;const float v0=sampleY*.25f+.015f,v1=(sampleY+1)*.25f-.015f;
    particle.position=glm::vec3(block)+glm::vec3(.5f)+(glm::vec3{random01(),random01(),random01()}-.5f)*.45f;particle.velocity=direction*(1.5f+random01()*2.3f);particle.axis={random01()-.5f,random01()-.5f,random01()-.5f};if(glm::dot(particle.axis,particle.axis)<.01f)particle.axis={0,1,0};else particle.axis=glm::normalize(particle.axis);particle.uvRegion={u0,v0,u1,v1};particle.age=0;particle.lifetime=.4f+random01()*.2f;particle.angle=random01()*6.2831853f;particle.angularVelocity=(random01()*2.f-1.f)*10.f;particle.size=.09f+random01()*.08f;particle.active=true;
  }
}
void ParticleSystem::update(float dt){for(auto& particle:m_particles)if(particle.active){particle.age+=std::max(0.f,dt);if(particle.age>=particle.lifetime){particle.active=false;continue;}particle.velocity.y-=13.f*dt;particle.position+=particle.velocity*dt;particle.angle+=particle.angularVelocity*dt;}}
void ParticleSystem::render(const glm::mat4& view,const glm::mat4& projection,const Texture& atlas,float daylight){
  m_shader.use();m_shader.setMat4("view",view);m_shader.setMat4("projection",projection);m_shader.setInt("atlas",0);m_shader.setFloat("daylight",daylight);atlas.bind();glBindVertexArray(m_vao);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glDepthMask(GL_FALSE);
  for(const auto& particle:m_particles)if(particle.active){const float remaining=1.f-particle.age/particle.lifetime;const float alpha=std::clamp(remaining/.35f,0.f,1.f);glm::mat4 model=glm::translate(glm::mat4(1),particle.position)*glm::rotate(glm::mat4(1),particle.angle,particle.axis)*glm::scale(glm::mat4(1),glm::vec3(particle.size));m_shader.setMat4("model",model);m_shader.setVec4("uvRegion",particle.uvRegion);m_shader.setFloat("alpha",alpha);glDrawArrays(GL_TRIANGLES,0,36);}
  glDepthMask(GL_TRUE);glDisable(GL_BLEND);
}
std::size_t ParticleSystem::activeCount()const{return static_cast<std::size_t>(std::count_if(m_particles.begin(),m_particles.end(),[](const Particle&p){return p.active;}));}
