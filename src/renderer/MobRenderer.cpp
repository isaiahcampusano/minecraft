#include "MobRenderer.h"
#include "CuboidMesh.h"
#include "../core/DayNightCycle.h"
#include "../world/PassiveMobSystem.h"
#include "../world/World.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>
#include <cmath>

namespace{
const char* VS=R"(#version 330 core
layout(location=0)in vec3 p;layout(location=1)in vec3 normal;layout(location=2)in vec3 shade;
out vec3 vNormal;out vec3 vShade;out vec3 vWorld;uniform mat4 projection,view,model;
void main(){vec4 world=model*vec4(p,1);vWorld=world.xyz;vNormal=mat3(transpose(inverse(model)))*normal;vShade=shade;gl_Position=projection*view*world;})";
const char* FS=R"(#version 330 core
in vec3 vNormal;in vec3 vShade;in vec3 vWorld;out vec4 outColor;uniform vec3 lightDir,viewPos,fogColor,baseColor;uniform float daylight,hurt;
void main(){vec3 n=normalize(vNormal);float diffuse=max(dot(n,normalize(lightDir)),0.0);float rim=pow(1.0-max(dot(n,normalize(viewPos-vWorld)),0.0),3.0)*.08;vec3 color=mix(baseColor,vec3(1,.12,.12),hurt)*vShade;vec3 lit=color*(mix(.15,.38,daylight)+diffuse*.68*daylight+rim);float d=length(viewPos-vWorld);float fog=clamp(1.0-exp(-.00055*d*d),0.0,.9);outColor=vec4(mix(lit,fogColor,fog),1);})";
std::array<glm::vec3,6> shades(){return{glm::vec3{.82f},glm::vec3{.7f},glm::vec3{1.f},glm::vec3{.52f},glm::vec3{.94f},glm::vec3{.64f}};}
glm::vec3 color(MobType type){return type==MobType::COW?glm::vec3(.38f,.2f,.09f):(type==MobType::PIG?glm::vec3(.94f,.5f,.58f):glm::vec3(.82f,.82f,.78f));}
}

MobRenderer::MobRenderer():m_shader(VS,FS){m_body=std::make_unique<CuboidMesh>(glm::vec3{.85f,.65f,1.25f},shades());m_wool=std::make_unique<CuboidMesh>(glm::vec3{.94f,.74f,1.34f},shades());m_head=std::make_unique<CuboidMesh>(glm::vec3{.58f,.58f,.58f},shades());m_leg=std::make_unique<CuboidMesh>(glm::vec3{.2f,.65f,.2f},shades());m_snout=std::make_unique<CuboidMesh>(glm::vec3{.42f,.22f,.18f},shades());}
MobRenderer::~MobRenderer()=default;
void MobRenderer::drawPart(const CuboidMesh& mesh,const glm::mat4& model,const glm::vec3& color,float hurt){m_shader.setMat4("model",model);m_shader.setVec3("baseColor",color);m_shader.setFloat("hurt",hurt);mesh.draw();}

void MobRenderer::draw(const PassiveMobSystem& system,const World& world,const glm::mat4& view,const glm::mat4& projection,const glm::vec3& camera,const DayNightCycle& dayNight){m_shader.use();m_shader.setMat4("view",view);m_shader.setMat4("projection",projection);m_shader.setVec3("lightDir",dayNight.sunDirection());m_shader.setVec3("viewPos",camera);m_shader.setVec3("fogColor",dayNight.skyBottom());m_shader.setFloat("daylight",dayNight.daylight());for(const auto& mob:system.mobs()){if(glm::distance(mob.position,camera)>PassiveMobSystem::ACTIVE_DISTANCE||!world.isChunkLoadedAt(static_cast<int>(std::floor(mob.position.x)),static_cast<int>(std::floor(mob.position.z))))continue;const float scale=mobScale(mob),heightScale=mobHeight(mob.type)/1.4f,hurt=mob.hurtTimer>0?std::min(1.f,mob.hurtTimer*4.f):0.f;const glm::mat4 base=glm::translate(glm::mat4(1),mob.position)*glm::rotate(glm::mat4(1),mob.yaw,{0,1,0})*glm::scale(glm::mat4(1),glm::vec3(scale,scale*heightScale,scale));const glm::vec3 bodyColor=color(mob.type),skinColor=mob.type==MobType::SHEEP?glm::vec3(.42f,.42f,.4f):bodyColor;drawPart(*m_body,base*glm::translate(glm::mat4(1),{0,.82f,0}),bodyColor,hurt);if(mob.type==MobType::SHEEP)drawPart(*m_wool,base*glm::translate(glm::mat4(1),{0,.82f,0}),{.92f,.92f,.88f},hurt);const bool grazing=mob.state==MobAIState::GRAZING&&mob.pathIndex>=mob.path.size();const float headY=grazing?.48f:1.02f,headPitch=grazing?-1.05f:0.f;const glm::mat4 headJoint=base*glm::translate(glm::mat4(1),{0,headY,-.77f})*glm::rotate(glm::mat4(1),headPitch,{1,0,0});drawPart(*m_head,headJoint,skinColor,hurt);drawPart(*m_snout,headJoint*glm::translate(glm::mat4(1),{0,-.08f,-.37f}),mob.type==MobType::PIG?glm::vec3(.75f,.32f,.38f):skinColor*.78f,hurt);const float swing=std::sin(mob.animationTime)*.55f;for(int i=0;i<4;++i){const float x=i%2==0?-.27f:.27f,z=i<2?-.4f:.4f,angle=(i==0||i==3)?swing:-swing;const glm::mat4 joint=base*glm::translate(glm::mat4(1),{x,.55f,z})*glm::rotate(glm::mat4(1),angle,{1,0,0});drawPart(*m_leg,joint*glm::translate(glm::mat4(1),{0,-.325f,0}),skinColor*.75f,hurt);}}}
