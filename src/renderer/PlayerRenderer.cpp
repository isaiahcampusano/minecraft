#include "PlayerRenderer.h"
#include "../player/Player.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
namespace{
const char* VS=R"(#version 330 core
layout(location=0)in vec3 p;layout(location=1)in vec3 c;out vec3 color;uniform mat4 projection,view,model;void main(){color=c;gl_Position=projection*view*model*vec4(p,1);})";
const char* FS=R"(#version 330 core
in vec3 color;out vec4 outColor;void main(){outColor=vec4(color,1);})";
void cube(std::vector<float>&v,glm::vec3 lo,glm::vec3 hi,glm::vec3 c){const int faces[6][4]={{1,5,6,2},{0,3,7,4},{4,5,6,7},{0,1,2,3},{2,6,7,3},{0,4,5,1}};glm::vec3 p[8]={{lo.x,lo.y,lo.z},{hi.x,lo.y,lo.z},{hi.x,lo.y,hi.z},{lo.x,lo.y,hi.z},{lo.x,hi.y,lo.z},{hi.x,hi.y,lo.z},{hi.x,hi.y,hi.z},{lo.x,hi.y,hi.z}};const int tri[6]={0,1,2,0,2,3};for(auto&f:faces)for(int n:tri){auto q=p[f[n]];v.insert(v.end(),{q.x,q.y,q.z,c.r,c.g,c.b});}}
}
PlayerRenderer::PlayerRenderer():m_shader(VS,FS){std::vector<float>v;cube(v,{-.25f,1.3f,-.25f},{.25f,1.8f,.25f},{.76f,.55f,.38f});cube(v,{-.25f,.65f,-.15f},{.25f,1.3f,.15f},{.1f,.55f,.68f});cube(v,{-.42f,.65f,-.12f},{-.25f,1.28f,.12f},{.76f,.55f,.38f});cube(v,{.25f,.65f,-.12f},{.42f,1.28f,.12f},{.76f,.55f,.38f});cube(v,{-.23f,0,-.14f},{-.02f,.65f,.14f},{.15f,.22f,.58f});cube(v,{.02f,0,-.14f},{.23f,.65f,.14f},{.15f,.22f,.58f});m_count=static_cast<GLsizei>(v.size()/6);glGenVertexArrays(1,&m_vao);glGenBuffers(1,&m_vbo);glBindVertexArray(m_vao);glBindBuffer(GL_ARRAY_BUFFER,m_vbo);glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(v.size()*sizeof(float)),v.data(),GL_STATIC_DRAW);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),nullptr);glEnableVertexAttribArray(0);glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),reinterpret_cast<void*>(3*sizeof(float)));glEnableVertexAttribArray(1);glBindVertexArray(0);}
PlayerRenderer::~PlayerRenderer(){if(m_vbo)glDeleteBuffers(1,&m_vbo);if(m_vao)glDeleteVertexArrays(1,&m_vao);}
void PlayerRenderer::draw(const Player&p,const glm::mat4&v,const glm::mat4&projection){m_shader.use();m_shader.setMat4("view",v);m_shader.setMat4("projection",projection);glm::mat4 model=glm::translate(glm::mat4(1),p.position)*glm::rotate(glm::mat4(1),glm::radians(-p.yaw-90.f),glm::vec3{0,1,0});m_shader.setMat4("model",model);glBindVertexArray(m_vao);glDrawArrays(GL_TRIANGLES,0,m_count);}
