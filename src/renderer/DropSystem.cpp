#include "DropSystem.h"
#include "Texture.h"
#include "../player/FoodRegistry.h"
#include "../player/ToolRegistry.h"
#include "../world/BlockRegistry.h"
#include "../world/World.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {
const char* DROP_VS=R"(#version 330 core
layout(location=0)in vec3 p;layout(location=1)in vec2 uv;layout(location=2)in float shade;
out vec2 vUV;out float vShade;uniform mat4 projection,view,model;uniform vec4 uvRegion;
void main(){vUV=mix(uvRegion.xy,uvRegion.zw,uv);vShade=shade;gl_Position=projection*view*model*vec4(p,1.0);})";
const char* DROP_FS=R"(#version 330 core
in vec2 vUV;in float vShade;out vec4 color;uniform sampler2D atlas;uniform int textured;uniform vec3 tint;uniform float daylight;
void main(){vec4 base=textured!=0?texture(atlas,vUV):vec4(tint,1.0);color=vec4(base.rgb*vShade*mix(0.2,1.0,daylight),base.a);})";
struct DropVertex{float x,y,z,u,v,shade;};

glm::vec3 itemTint(const ItemStack& stack){
  ItemColor color{255,0,255};
  if(stack.kind==ItemKind::MATERIAL)color=materialColor(stack.materialType);
  else if(stack.kind==ItemKind::TOOL)color=toolColor(stack.toolKind,stack.toolTier);
  else if(stack.kind==ItemKind::FOOD)color=foodColor(stack.foodType);
  return {color.r/255.f,color.g/255.f,color.b/255.f};
}
}

DropSystem::DropSystem():m_shader(DROP_VS,DROP_FS){
  static constexpr float corners[8][3]={{-.5f,-.5f,-.5f},{.5f,-.5f,-.5f},{.5f,-.5f,.5f},{-.5f,-.5f,.5f},{-.5f,.5f,-.5f},{.5f,.5f,-.5f},{.5f,.5f,.5f},{-.5f,.5f,.5f}};
  static constexpr int faces[6][4]={{1,5,6,2},{0,3,7,4},{4,7,6,5},{0,1,2,3},{2,6,7,3},{0,4,5,1}};
  static constexpr float shades[6]={.82f,.72f,1.f,.55f,.9f,.68f};
  static constexpr float uvs[4][2]={{0,0},{1,0},{1,1},{0,1}};
  std::array<DropVertex,36> vertices{};std::size_t out=0;
  for(int face=0;face<6;++face)for(int index:{0,1,2,0,2,3}){const int corner=faces[face][index];vertices[out++]={corners[corner][0],corners[corner][1],corners[corner][2],uvs[index][0],uvs[index][1],shades[face]};}
  glGenVertexArrays(1,&m_vao);glGenBuffers(1,&m_vbo);glBindVertexArray(m_vao);glBindBuffer(GL_ARRAY_BUFFER,m_vbo);glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices.data(),GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(DropVertex),reinterpret_cast<void*>(offsetof(DropVertex,x)));glEnableVertexAttribArray(0);
  glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,sizeof(DropVertex),reinterpret_cast<void*>(offsetof(DropVertex,u)));glEnableVertexAttribArray(1);
  glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,sizeof(DropVertex),reinterpret_cast<void*>(offsetof(DropVertex,shade)));glEnableVertexAttribArray(2);glBindVertexArray(0);
}

DropSystem::~DropSystem(){if(m_vbo)glDeleteBuffers(1,&m_vbo);if(m_vao)glDeleteVertexArrays(1,&m_vao);}
float DropSystem::random01(){m_randomState^=m_randomState<<13;m_randomState^=m_randomState>>17;m_randomState^=m_randomState<<5;return(m_randomState&0x00ffffffu)/static_cast<float>(0x01000000u);}
DropSystem::Drop& DropSystem::acquire(){for(auto& drop:m_drops)if(!drop.active)return drop;return *std::max_element(m_drops.begin(),m_drops.end(),[](const Drop&a,const Drop&b){return a.age<b.age;});}

void DropSystem::spawn(const glm::vec3& position,const ItemStack& stack){
  if(stack.empty())return;
  auto& drop=acquire();drop.position=position+glm::vec3(.5f,.45f,.5f);drop.velocity={(random01()-.5f)*.5f,2.2f,(random01()-.5f)*.5f};drop.stack=stack;drop.age=0;drop.spin=random01()*6.2831853f;drop.pickupDelay=.4f;drop.onGround=false;drop.active=true;
}

void DropSystem::update(float dt,const World& world,const glm::vec3& playerPosition,const PickupHandler& tryPickup){
  dt=std::max(0.f,dt);
  for(auto& drop:m_drops){
    if(!drop.active)continue;
    drop.age+=dt;drop.spin+=dt*1.4f;drop.pickupDelay=std::max(0.f,drop.pickupDelay-dt);
    if(drop.age>=MAX_AGE){drop.active=false;continue;}
    if(drop.onGround){const int bx=static_cast<int>(std::floor(drop.position.x)),by=static_cast<int>(std::floor(drop.position.y-HALF_SIZE-.01f)),bz=static_cast<int>(std::floor(drop.position.z));if(!isSolid(world.getBlock(bx,by,bz)))drop.onGround=false;}
    if(!drop.onGround){drop.velocity.y-=13.f*dt;const glm::vec3 next=drop.position+drop.velocity*dt;const int bx=static_cast<int>(std::floor(next.x)),by=static_cast<int>(std::floor(next.y-HALF_SIZE)),bz=static_cast<int>(std::floor(next.z));if(drop.velocity.y<=0&&isSolid(world.getBlock(bx,by,bz))){drop.position={next.x,static_cast<float>(by+1)+HALF_SIZE,next.z};drop.velocity={0,0,0};drop.onGround=true;}else drop.position=next;}
    if(drop.pickupDelay<=0&&glm::length(playerPosition-drop.position)<PICKUP_RADIUS&&tryPickup&&tryPickup(drop.stack))drop.active=false;
  }
}

void DropSystem::render(const glm::mat4& view,const glm::mat4& projection,const Texture& atlas,float daylight){
  m_shader.use();m_shader.setMat4("view",view);m_shader.setMat4("projection",projection);m_shader.setInt("atlas",0);m_shader.setFloat("daylight",daylight);atlas.bind();glBindVertexArray(m_vao);
  for(const auto& drop:m_drops)if(drop.active){const bool block=drop.stack.kind==ItemKind::BLOCK;const float tileWidth=1.f/static_cast<float>(BLOCK_TYPE_COUNT),tile=block?static_cast<float>(drop.stack.blockType):0.f;const glm::vec4 uv{tile*tileWidth+.02f,.02f,(tile+1.f)*tileWidth-.02f,.98f};const float bob=drop.onGround?std::sin(drop.age*BOB_SPEED)*BOB_HEIGHT:0.f;const glm::mat4 model=glm::translate(glm::mat4(1),drop.position+glm::vec3(0,bob,0))*glm::rotate(glm::mat4(1),drop.spin,glm::vec3(0,1,0))*glm::scale(glm::mat4(1),glm::vec3(.25f));m_shader.setMat4("model",model);m_shader.setVec4("uvRegion",uv);m_shader.setInt("textured",block?1:0);m_shader.setVec3("tint",itemTint(drop.stack));glDrawArrays(GL_TRIANGLES,0,36);}
}

std::size_t DropSystem::activeCount()const{return static_cast<std::size_t>(std::count_if(m_drops.begin(),m_drops.end(),[](const Drop&drop){return drop.active;}));}
