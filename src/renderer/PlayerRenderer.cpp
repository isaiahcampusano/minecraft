#include "PlayerRenderer.h"
#include "CuboidMesh.h"
#include "../player/Player.h"
#include "../core/DayNightCycle.h"
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <cmath>

namespace{
const char* VS=R"(#version 330 core
layout(location=0)in vec3 p;layout(location=1)in vec3 normal;layout(location=2)in vec3 color;
out vec3 vNormal;out vec3 vColor;out vec3 vWorld;uniform mat4 projection,view,model;
void main(){vec4 world=model*vec4(p,1);vWorld=world.xyz;vNormal=mat3(transpose(inverse(model)))*normal;vColor=color;gl_Position=projection*view*world;})";
const char* FS=R"(#version 330 core
in vec3 vNormal;in vec3 vColor;in vec3 vWorld;out vec4 outColor;uniform vec3 lightDir,viewPos,fogColor;uniform float daylight;
void main(){vec3 n=normalize(vNormal);float diffuse=max(dot(n,normalize(lightDir)),0.0);float rim=pow(1.0-max(dot(n,normalize(viewPos-vWorld)),0.0),3.0)*.08;vec3 lit=vColor*(mix(.15,.38,daylight)+diffuse*.68*daylight+rim);float d=length(viewPos-vWorld);float fog=clamp(1.0-exp(-.00055*d*d),0.0,.9);outColor=vec4(mix(lit,fogColor,fog),1);})";
std::array<glm::vec3,6> shades(glm::vec3 base){return{base*.82f,base*.7f,glm::min(base*1.16f,glm::vec3(1)),base*.52f,base*.94f,base*.64f};}
}

PlayerRenderer::PlayerRenderer():m_shader(VS,FS){
  m_head=std::make_unique<CuboidMesh>(glm::vec3{.5f,.5f,.5f},shades({.78f,.56f,.4f}));
  m_torso=std::make_unique<CuboidMesh>(glm::vec3{.5f,.65f,.3f},shades({.08f,.55f,.68f}));
  m_arm=std::make_unique<CuboidMesh>(glm::vec3{.22f,.65f,.22f},shades({.78f,.56f,.4f}));
  m_leg=std::make_unique<CuboidMesh>(glm::vec3{.23f,.65f,.24f},shades({.12f,.2f,.55f}));
  m_eye=std::make_unique<CuboidMesh>(glm::vec3{.085f,.07f,.018f},shades({.04f,.05f,.06f}));
  m_mouth=std::make_unique<CuboidMesh>(glm::vec3{.17f,.04f,.018f},shades({.28f,.08f,.06f}));
}
PlayerRenderer::~PlayerRenderer()=default;
void PlayerRenderer::drawPart(const CuboidMesh& mesh,const glm::mat4& model){m_shader.setMat4("model",model);mesh.draw();}
void PlayerRenderer::draw(const Player&p,float dt,const glm::mat4&view,const glm::mat4&projection,const glm::vec3&camera,const DayNightCycle&dayNight){
  float speed=std::sqrt(p.velocity.x*p.velocity.x+p.velocity.z*p.velocity.z);float cameraYaw=glm::radians(-p.yaw-90.f);if(!m_hasBodyYaw){m_bodyYaw=cameraYaw;m_hasBodyYaw=true;}if(speed>.1f){float desired=std::atan2(-p.velocity.x,-p.velocity.z);float delta=std::atan2(std::sin(desired-m_bodyYaw),std::cos(desired-m_bodyYaw));m_bodyYaw+=delta*(1.f-std::exp(-10.f*dt));}
  m_animator.update(p,dt,m_bodyYaw);const PlayerPose&a=m_animator.pose();m_shader.use();m_shader.setMat4("view",view);m_shader.setMat4("projection",projection);m_shader.setVec3("lightDir",dayNight.sunDirection());m_shader.setVec3("viewPos",camera);m_shader.setVec3("fogColor",dayNight.skyBottom());m_shader.setFloat("daylight",dayNight.daylight());
  glm::mat4 base=glm::translate(glm::mat4(1),p.position)*glm::rotate(glm::mat4(1),m_bodyYaw,{0,1,0});
  glm::mat4 body=base*glm::translate(glm::mat4(1),{0,.65f+a.torsoBob,0})*glm::rotate(glm::mat4(1),a.torsoSway,{0,0,1})*glm::rotate(glm::mat4(1),a.torsoLean,{1,0,0})*glm::translate(glm::mat4(1),{0,-.65f,0});
  drawPart(*m_torso,body*glm::translate(glm::mat4(1),{0,.975f,0}));
  glm::mat4 headJoint=body*glm::translate(glm::mat4(1),{0,1.3f,0})*glm::rotate(glm::mat4(1),a.headYaw,{0,1,0})*glm::rotate(glm::mat4(1),a.headPitch,{1,0,0});
  drawPart(*m_head,headJoint*glm::translate(glm::mat4(1),{0,.25f,0}));
  drawPart(*m_eye,headJoint*glm::translate(glm::mat4(1),{-.12f,.31f,-.261f}));drawPart(*m_eye,headJoint*glm::translate(glm::mat4(1),{.12f,.31f,-.261f}));drawPart(*m_mouth,headJoint*glm::translate(glm::mat4(1),{0,.18f,-.261f}));
  auto limb=[&](float x,float y,float angle,float outward,const CuboidMesh&mesh){glm::mat4 joint=body*glm::translate(glm::mat4(1),{x,y,0})*glm::rotate(glm::mat4(1),outward,{0,0,1})*glm::rotate(glm::mat4(1),angle,{1,0,0});drawPart(mesh,joint*glm::translate(glm::mat4(1),{0,-.325f,0}));};
  limb(-.36f,1.29f,a.leftArm,a.leftArmOut,*m_arm);limb(.36f,1.29f,a.rightArm,a.rightArmOut,*m_arm);limb(-.12f,.65f,a.leftLeg,0,*m_leg);limb(.12f,.65f,a.rightLeg,0,*m_leg);
}
void PlayerRenderer::drawFirstPerson(const Player&p,const glm::mat4&view,const glm::mat4&projection,const glm::vec3&camera,const DayNightCycle&dayNight){
  const glm::vec3 forward=glm::normalize(p.forward());
  const glm::vec3 right=glm::normalize(glm::cross(forward,glm::vec3{0,1,0}));
  const glm::vec3 up=glm::normalize(glm::cross(right,forward));
  const glm::mat4 basis{
    glm::vec4(right,0),glm::vec4(up,0),glm::vec4(-forward,0),glm::vec4(0,0,0,1)
  };
  m_shader.use();m_shader.setMat4("view",view);m_shader.setMat4("projection",projection);
  m_shader.setVec3("lightDir",dayNight.sunDirection());m_shader.setVec3("viewPos",camera);
  m_shader.setVec3("fogColor",dayNight.skyBottom());m_shader.setFloat("daylight",dayNight.daylight());
  const glm::mat4 hand=glm::translate(glm::mat4(1),camera+forward*.62f+right*.43f-up*.45f)
    *basis*glm::rotate(glm::mat4(1),-.35f,{1,0,0});
  drawPart(*m_arm,hand*glm::scale(glm::mat4(1),{1.f,1.18f,1.f}));
}
