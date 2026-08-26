#include "Renderer.h"
#include "../utils/Raycaster.h"
#include "../world/World.h"
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <vector>

namespace {
const char* VS=R"(#version 330 core
layout(location=0)in vec3 p;layout(location=1)in vec2 uv;layout(location=2)in float shade;
out vec2 vUV;out float vShade;out vec3 vWorld;uniform mat4 projection,view;
void main(){vUV=uv;vShade=shade;vWorld=p;gl_Position=projection*view*vec4(p,1.0);})";
const char* FS=R"(#version 330 core
in vec2 vUV;in float vShade;in vec3 vWorld;out vec4 color;uniform sampler2D atlas;uniform vec3 cameraPos;uniform vec3 fogColor;
void main(){vec4 base=texture(atlas,vUV)*vec4(vec3(vShade),1.0);float d=length(vWorld-cameraPos);float fog=1.0-exp(-0.00055*d*d);color=mix(base,vec4(fogColor,1.0),clamp(fog,0.0,0.92));})";
const char* COLOR_VS=R"(#version 330 core
layout(location=0)in vec3 p;uniform mat4 transform;void main(){gl_Position=transform*vec4(p,1.0);})";
const char* COLOR_FS=R"(#version 330 core
out vec4 color;uniform vec4 tint;void main(){color=tint;})";
const char* SKY_VS=R"(#version 330 core
layout(location=0)in vec2 p;out float height;void main(){height=p.y;gl_Position=vec4(p,0.999,1.0);})";
const char* SKY_FS=R"(#version 330 core
in float height;out vec4 color;void main(){float t=height*.5+.5;color=vec4(mix(vec3(.70,.86,.96),vec3(.20,.52,.86),t),1.0);})";

std::array<unsigned char,7> glyph(char c){
  switch(c){
    case 'A':return{14,17,17,31,17,17,17};case 'B':return{30,17,17,30,17,17,30};case 'C':return{14,17,16,16,16,17,14};
    case 'D':return{30,17,17,17,17,17,30};case 'E':return{31,16,16,30,16,16,31};case 'F':return{31,16,16,30,16,16,16};
    case 'G':return{14,17,16,23,17,17,14};case 'I':return{14,4,4,4,4,4,14};case 'K':return{17,18,20,24,20,18,17};
    case 'L':return{16,16,16,16,16,16,31};case 'N':return{17,25,21,19,17,17,17};case 'O':return{14,17,17,17,17,17,14};
    case 'P':return{30,17,17,30,16,16,16};case 'R':return{30,17,17,30,20,18,17};case 'S':return{15,16,16,14,1,1,30};
    case 'T':return{31,4,4,4,4,4,4};case 'Y':return{17,17,10,4,4,4,4};case '0':return{14,17,19,21,25,17,14};
    case '1':return{4,12,4,4,4,4,14};case '2':return{14,17,1,2,4,8,31};case '3':return{30,1,1,14,1,1,30};
    case '4':return{2,6,10,18,31,2,2};case '5':return{31,16,16,30,1,1,30};case '6':return{14,16,16,30,17,17,14};
    case '7':return{31,1,2,4,8,8,8};case '8':return{14,17,17,14,17,17,14};case '9':return{14,17,17,15,1,1,14};
    case ':':return{0,4,4,0,4,4,0};case '.':return{0,0,0,0,0,4,4};case '-':return{0,0,0,31,0,0,0};default:return{};
  }
}
}

Renderer::Renderer():m_shader(VS,FS),m_colorShader(COLOR_VS,COLOR_FS),m_skyShader(SKY_VS,SKY_FS){
  glGenVertexArrays(1,&m_lineVao);glGenBuffers(1,&m_lineVbo);glBindVertexArray(m_lineVao);glBindBuffer(GL_ARRAY_BUFFER,m_lineVbo);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);glEnableVertexAttribArray(0);
  const float sky[]={-1,-1,1,-1,-1,1,1,1};glGenVertexArrays(1,&m_skyVao);glGenBuffers(1,&m_skyVbo);glBindVertexArray(m_skyVao);glBindBuffer(GL_ARRAY_BUFFER,m_skyVbo);glBufferData(GL_ARRAY_BUFFER,sizeof(sky),sky,GL_STATIC_DRAW);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);glEnableVertexAttribArray(0);glBindVertexArray(0);
}
Renderer::~Renderer(){if(m_skyVbo)glDeleteBuffers(1,&m_skyVbo);if(m_skyVao)glDeleteVertexArrays(1,&m_skyVao);if(m_lineVbo)glDeleteBuffers(1,&m_lineVbo);if(m_lineVao)glDeleteVertexArrays(1,&m_lineVao);}
void Renderer::drawSky(){glDisable(GL_DEPTH_TEST);m_skyShader.use();glBindVertexArray(m_skyVao);glDrawArrays(GL_TRIANGLE_STRIP,0,4);glEnable(GL_DEPTH_TEST);}
void Renderer::drawOutline(const RayHit& hit,const glm::mat4& view,const glm::mat4& projection){if(!hit.hit)return;constexpr float e=.002f;float x=hit.block.x-e,y=hit.block.y-e,z=hit.block.z-e,s=1.f+2*e;
  const float p[]={x,y,z,x+s,y,z, x+s,y,z,x+s,y+s,z, x+s,y+s,z,x,y+s,z, x,y+s,z,x,y,z,
    x,y,z+s,x+s,y,z+s, x+s,y,z+s,x+s,y+s,z+s, x+s,y+s,z+s,x,y+s,z+s, x,y+s,z+s,x,y,z+s,
    x,y,z,x,y,z+s, x+s,y,z,x+s,y,z+s, x+s,y+s,z,x+s,y+s,z+s, x,y+s,z,x,y+s,z+s};
  glBindBuffer(GL_ARRAY_BUFFER,m_lineVbo);glBufferData(GL_ARRAY_BUFFER,sizeof(p),p,GL_DYNAMIC_DRAW);m_colorShader.use();m_colorShader.setMat4("transform",projection*view);m_colorShader.setVec4("tint",{.05f,.05f,.05f,1});glLineWidth(2.5f);glBindVertexArray(m_lineVao);glDrawArrays(GL_LINES,0,24);
}
void Renderer::drawOverlay(int width,int height,const std::string& text){
  std::vector<float> p;auto quad=[&](float x,float y,float w,float h){float q[]={x,y,0,x+w,y,0,x+w,y+h,0,x,y,0,x+w,y+h,0,x,y+h,0};p.insert(p.end(),q,q+18);};
  float cx=width*.5f,cy=height*.5f;quad(cx-9,cy-1,18,2);quad(cx-1,cy-9,2,18);
  float penX=12,penY=height-18;for(char c:text){if(c=='\n'){penX=12;penY-=18;continue;}auto rows=glyph(c);for(int row=0;row<7;++row)for(int col=0;col<5;++col)if(rows[row]&(1<<(4-col)))quad(penX+col*2,penY-row*2,2,2);penX+=12;}
  glBindBuffer(GL_ARRAY_BUFFER,m_lineVbo);glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(p.size()*sizeof(float)),p.data(),GL_DYNAMIC_DRAW);m_colorShader.use();m_colorShader.setMat4("transform",glm::ortho(0.f,static_cast<float>(width),0.f,static_cast<float>(height)));m_colorShader.setVec4("tint",{.05f,.05f,.05f,.9f});glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glBindVertexArray(m_lineVao);glDrawArrays(GL_TRIANGLES,0,static_cast<GLsizei>(p.size()/3));glEnable(GL_CULL_FACE);glEnable(GL_DEPTH_TEST);
}
void Renderer::draw(const World&w,const glm::mat4&v,const glm::mat4&p,const glm::vec3&camera,const RayHit&hit,int width,int height,const std::string&hud){drawSky();m_shader.use();m_shader.setMat4("view",v);m_shader.setMat4("projection",p);m_shader.setInt("atlas",0);m_shader.setVec3("cameraPos",camera);m_shader.setVec3("fogColor",{.70f,.86f,.96f});m_texture.bind();w.render();drawOutline(hit,v,p);drawOverlay(width,height,hud);}
