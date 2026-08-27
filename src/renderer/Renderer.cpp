#include "Renderer.h"
#include "../utils/Raycaster.h"
#include "../world/World.h"
#include "PlayerRenderer.h"
#include "../player/Player.h"
#include "../player/Inventory.h"
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
  const float sky[]={-1,-1,1,-1,-1,1,1,1};glGenVertexArrays(1,&m_skyVao);glGenBuffers(1,&m_skyVbo);glBindVertexArray(m_skyVao);glBindBuffer(GL_ARRAY_BUFFER,m_skyVbo);glBufferData(GL_ARRAY_BUFFER,sizeof(sky),sky,GL_STATIC_DRAW);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);glEnableVertexAttribArray(0);glBindVertexArray(0);m_playerRenderer=std::make_unique<PlayerRenderer>();
}
Renderer::~Renderer(){m_playerRenderer.reset();if(m_skyVbo)glDeleteBuffers(1,&m_skyVbo);if(m_skyVao)glDeleteVertexArrays(1,&m_skyVao);if(m_lineVbo)glDeleteBuffers(1,&m_lineVbo);if(m_lineVao)glDeleteVertexArrays(1,&m_lineVao);}
void Renderer::drawSky(){glDisable(GL_DEPTH_TEST);m_skyShader.use();glBindVertexArray(m_skyVao);glDrawArrays(GL_TRIANGLE_STRIP,0,4);glEnable(GL_DEPTH_TEST);}
void Renderer::drawOutline(const RayHit& hit,const glm::mat4& view,const glm::mat4& projection){if(!hit.hit)return;constexpr float e=.002f;float x=hit.block.x-e,y=hit.block.y-e,z=hit.block.z-e,s=1.f+2*e;
  const float p[]={x,y,z,x+s,y,z, x+s,y,z,x+s,y+s,z, x+s,y+s,z,x,y+s,z, x,y+s,z,x,y,z,
    x,y,z+s,x+s,y,z+s, x+s,y,z+s,x+s,y+s,z+s, x+s,y+s,z+s,x,y+s,z+s, x,y+s,z+s,x,y,z+s,
    x,y,z,x,y,z+s, x+s,y,z,x+s,y,z+s, x+s,y+s,z,x+s,y+s,z+s, x,y+s,z,x,y+s,z+s};
  glBindBuffer(GL_ARRAY_BUFFER,m_lineVbo);glBufferData(GL_ARRAY_BUFFER,sizeof(p),p,GL_DYNAMIC_DRAW);m_colorShader.use();m_colorShader.setMat4("transform",projection*view);m_colorShader.setVec4("tint",{.05f,.05f,.05f,1});glLineWidth(2.5f);glBindVertexArray(m_lineVao);glDrawArrays(GL_LINES,0,24);
}
void Renderer::drawOverlay(int width,int height,const std::string& text,const Inventory& inventory){
  const glm::mat4 screen=glm::ortho(0.f,static_cast<float>(width),0.f,static_cast<float>(height));
  auto drawQuads=[&](const std::vector<float>& vertices,const glm::vec4& color){if(vertices.empty())return;glBindBuffer(GL_ARRAY_BUFFER,m_lineVbo);glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(float)),vertices.data(),GL_DYNAMIC_DRAW);m_colorShader.use();m_colorShader.setMat4("transform",screen);m_colorShader.setVec4("tint",color);glBindVertexArray(m_lineVao);glDrawArrays(GL_TRIANGLES,0,static_cast<GLsizei>(vertices.size()/3));};
  auto appendQuad=[](std::vector<float>& vertices,float x,float y,float w,float h){float q[]={x,y,0,x+w,y,0,x+w,y+h,0,x,y,0,x+w,y+h,0,x,y+h,0};vertices.insert(vertices.end(),q,q+18);};
  glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);
  std::vector<float> p;auto quad=[&](float x,float y,float w,float h){appendQuad(p,x,y,w,h);};
  float cx=width*.5f,cy=height*.5f;quad(cx-9,cy-1,18,2);quad(cx-1,cy-9,2,18);
  float penX=12,penY=height-18;for(char c:text){if(c=='\n'){penX=12;penY-=18;continue;}auto rows=glyph(c);for(int row=0;row<7;++row)for(int col=0;col<5;++col)if(rows[row]&(1<<(4-col)))quad(penX+col*2,penY-row*2,2,2);penX+=12;}
  drawQuads(p,{.05f,.05f,.05f,.9f});

  constexpr float slotSize=48.f,gap=4.f,total=Inventory::SLOT_COUNT*slotSize+(Inventory::SLOT_COUNT-1)*gap;
  const float startX=(width-total)*.5f,slotY=16.f;
  std::vector<float> borders,selectedBorder,slots;
  for(int i=0;i<Inventory::SLOT_COUNT;++i){float x=startX+i*(slotSize+gap);if(i==inventory.selectedSlot())appendQuad(selectedBorder,x-4,slotY-4,slotSize+8,slotSize+8);else appendQuad(borders,x-2,slotY-2,slotSize+4,slotSize+4);appendQuad(slots,x,slotY,slotSize,slotSize);}
  drawQuads(borders,{.38f,.38f,.40f,.95f});drawQuads(selectedBorder,{.95f,.95f,.88f,1.f});drawQuads(slots,{.16f,.16f,.18f,.94f});
  constexpr float colors[9][3]={{105.f/255,178.f/255,62.f/255},{126.f/255,78.f/255,43.f/255},{58.f/255,58.f/255,60.f/255},{122.f/255,122.f/255,122.f/255},{102.f/255,102.f/255,106.f/255},{171.f/255,132.f/255,76.f/255},{219.f/255,207.f/255,155.f/255},{177.f/255,218.f/255,224.f/255},{88.f/255,147.f/255,52.f/255}};
  for(int i=0;i<Inventory::SLOT_COUNT;++i){const auto& stack=inventory.slot(i);if(stack.count<=0||stack.type==BlockType::AIR)continue;float x=startX+i*(slotSize+gap);std::vector<float> swatch;appendQuad(swatch,x+8,slotY+8,slotSize-16,slotSize-16);int colorIndex=static_cast<int>(stack.type)-1;drawQuads(swatch,{colors[colorIndex][0],colors[colorIndex][1],colors[colorIndex][2],1.f});std::string count=std::to_string(stack.count);std::vector<float> digits;float digitX=x+slotSize-4.f-static_cast<float>(count.size())*8.f;for(char c:count){auto rows=glyph(c);for(int row=0;row<7;++row)for(int col=0;col<5;++col)if(rows[row]&(1<<(4-col)))appendQuad(digits,digitX+col,slotY+4+(6-row),1,1);digitX+=8;}drawQuads(digits,{1,1,1,1});}
  glEnable(GL_CULL_FACE);glEnable(GL_DEPTH_TEST);
}
void Renderer::draw(const World&w,const Player&player,bool showPlayer,float dt,const glm::mat4&v,const glm::mat4&p,const glm::vec3&camera,const RayHit&hit,int width,int height,const std::string&hud,const Inventory&inventory){drawSky();m_shader.use();m_shader.setMat4("view",v);m_shader.setMat4("projection",p);m_shader.setInt("atlas",0);m_shader.setVec3("cameraPos",camera);m_shader.setVec3("fogColor",{.70f,.86f,.96f});m_texture.bind();w.render();if(showPlayer)m_playerRenderer->draw(player,dt,v,p,camera);drawOutline(hit,v,p);drawOverlay(width,height,hud,inventory);}
