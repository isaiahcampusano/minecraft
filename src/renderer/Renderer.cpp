#include "Renderer.h"
#include "../world/World.h"
namespace {
const char* VS=R"(#version 330 core
layout(location=0)in vec3 p;layout(location=1)in vec2 uv;layout(location=2)in float shade;
out vec2 vUV;out float vShade;uniform mat4 projection,view;
void main(){vUV=uv;vShade=shade;gl_Position=projection*view*vec4(p,1.0);})";
const char* FS=R"(#version 330 core
in vec2 vUV;in float vShade;out vec4 color;uniform sampler2D atlas;
void main(){color=texture(atlas,vUV)*vec4(vec3(vShade),1.0);})";
}
Renderer::Renderer():m_shader(VS,FS){}
void Renderer::draw(const World&w,const glm::mat4&v,const glm::mat4&p){m_shader.use();m_shader.setMat4("view",v);m_shader.setMat4("projection",p);m_shader.setInt("atlas",0);m_texture.bind();w.render();}
