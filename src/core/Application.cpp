#include "Application.h"
#include "../utils/Raycaster.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>

Application::Application(){
  if(!glfwInit())throw std::runtime_error("Could not initialize GLFW");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
  m_window=glfwCreateWindow(1280,720,"Minecraft Superflat",nullptr,nullptr);if(!m_window){glfwTerminate();throw std::runtime_error("Could not create window");}
  glfwMakeContextCurrent(m_window);if(!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)))throw std::runtime_error("Could not load OpenGL");
  m_renderer=std::make_unique<Renderer>();glfwSetWindowUserPointer(m_window,this);glfwSetCursorPosCallback(m_window,cursor);glfwSetScrollCallback(m_window,scroll);glfwSetMouseButtonCallback(m_window,mouseButton);glfwSetKeyCallback(m_window,key);
  glfwSetInputMode(m_window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);glfwSwapInterval(1);glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glCullFace(GL_BACK);
}
Application::~Application(){m_renderer.reset();if(m_window)glfwDestroyWindow(m_window);glfwTerminate();}
void Application::cursor(GLFWwindow*w,double x,double y){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));if(!a->m_captured)return;if(a->m_firstMouse){a->m_lastX=x;a->m_lastY=y;a->m_firstMouse=false;}a->m_player.look(static_cast<float>(x-a->m_lastX),static_cast<float>(a->m_lastY-y));a->m_lastX=x;a->m_lastY=y;}
void Application::scroll(GLFWwindow*w,double,double y){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));a->m_camera.fov=std::clamp(a->m_camera.fov-static_cast<float>(y)*2.f,30.f,90.f);}
void Application::key(GLFWwindow*w,int k,int,int action,int){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));if(action!=GLFW_PRESS)return;if(k==GLFW_KEY_ESCAPE){a->m_captured=!a->m_captured;a->m_firstMouse=true;glfwSetInputMode(w,GLFW_CURSOR,a->m_captured?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL);}else if(k==GLFW_KEY_F)a->m_player.toggleFly();else if(k==GLFW_KEY_F5)a->m_camera.togglePOV();}
void Application::mouseButton(GLFWwindow*w,int button,int action,int){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));if(action!=GLFW_PRESS||!a->m_captured||a->m_clickCooldown>0)return;glm::vec3 origin=a->m_camera.position(a->m_player),direction=a->m_camera.direction(a->m_player);auto hit=Raycaster::cast(a->m_world,origin,direction,a->m_camera.thirdPerson?12.f:6.f);if(!hit.hit)return;if(button==GLFW_MOUSE_BUTTON_LEFT&&hit.block.y>0)a->m_world.setBlock(hit.block.x,hit.block.y,hit.block.z,BlockType::AIR);if(button==GLFW_MOUSE_BUTTON_RIGHT&&!a->m_player.overlapsBlock(hit.adjacent))a->m_world.setBlock(hit.adjacent.x,hit.adjacent.y,hit.adjacent.z,BlockType::DIRT);a->m_clickCooldown=.12f;}
void Application::input(float){glm::vec3 forward=m_player.forward();forward.y=0;if(glm::length(forward)>0)forward=glm::normalize(forward);glm::vec3 right=glm::normalize(glm::cross(forward,glm::vec3{0,1,0}));glm::vec3 move{0};
  if(glfwGetKey(m_window,GLFW_KEY_W)==GLFW_PRESS)move+=forward;
  if(glfwGetKey(m_window,GLFW_KEY_S)==GLFW_PRESS)move-=forward;
  if(glfwGetKey(m_window,GLFW_KEY_D)==GLFW_PRESS)move+=right;
  if(glfwGetKey(m_window,GLFW_KEY_A)==GLFW_PRESS)move-=right;
  if(glm::length(move)>0)move=glm::normalize(move);
  float speed=glfwGetKey(m_window,GLFW_KEY_LEFT_CONTROL)==GLFW_PRESS?10.f:5.f;m_player.velocity.x=move.x*speed;m_player.velocity.z=move.z*speed;
  if(m_player.isFlying){m_player.velocity.y=0;if(glfwGetKey(m_window,GLFW_KEY_SPACE)==GLFW_PRESS)m_player.velocity.y=speed;if(glfwGetKey(m_window,GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS)m_player.velocity.y=-speed;}else if(glfwGetKey(m_window,GLFW_KEY_SPACE)==GLFW_PRESS)m_player.jump();
}
void Application::run(){double last=glfwGetTime();while(!glfwWindowShouldClose(m_window)){double now=glfwGetTime();float dt=std::min(static_cast<float>(now-last),.05f);last=now;m_clickCooldown=std::max(0.f,m_clickCooldown-dt);glfwPollEvents();m_world.update(m_player.position);input(dt);m_player.update(dt,m_world);
    int width,height;glfwGetFramebufferSize(m_window,&width,&height);glViewport(0,0,width,height);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glm::mat4 projection=glm::perspective(glm::radians(m_camera.fov),width/static_cast<float>(std::max(height,1)),.1f,500.f);glm::vec3 cameraPos=m_camera.position(m_player),cameraDir=m_camera.direction(m_player);RayHit target=Raycaster::cast(m_world,cameraPos,cameraDir,m_camera.thirdPerson?12.f:6.f);
    BlockType selected=target.hit?m_world.getBlock(target.block.x,target.block.y,target.block.z):BlockType::AIR;const char* name=selected==BlockType::GRASS?"GRASS":selected==BlockType::DIRT?"DIRT":selected==BlockType::BEDROCK?"BEDROCK":"NONE";
    std::ostringstream hud;hud<<std::fixed<<std::setprecision(1)<<"POS: "<<m_player.position.x<<" "<<m_player.position.y<<" "<<m_player.position.z<<"\nFLY: "<<(m_player.isFlying?"ON":"OFF")<<"\nTARGET: "<<name;
    m_renderer->draw(m_world,m_player,m_camera.thirdPerson,dt,m_camera.view(m_player),projection,cameraPos,target,width,height,hud.str());glfwSwapBuffers(m_window);}}
