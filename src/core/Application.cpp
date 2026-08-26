#include "Application.h"
#include "../utils/Raycaster.h"
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>
#include <stdexcept>

Application::Application(){
  if(!glfwInit())throw std::runtime_error("Could not initialize GLFW");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
  m_window=glfwCreateWindow(1280,720,"Minecraft Superflat",nullptr,nullptr);if(!m_window){glfwTerminate();throw std::runtime_error("Could not create window");}
  glfwMakeContextCurrent(m_window);if(!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)))throw std::runtime_error("Could not load OpenGL");
  m_renderer=std::make_unique<Renderer>();glfwSetWindowUserPointer(m_window,this);glfwSetCursorPosCallback(m_window,cursor);glfwSetScrollCallback(m_window,scroll);glfwSetMouseButtonCallback(m_window,mouseButton);glfwSetKeyCallback(m_window,key);
  glfwSetInputMode(m_window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);glfwSwapInterval(1);glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glCullFace(GL_BACK);glClearColor(.52f,.78f,.95f,1.f);
}
Application::~Application(){m_renderer.reset();if(m_window)glfwDestroyWindow(m_window);glfwTerminate();}
void Application::cursor(GLFWwindow*w,double x,double y){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));if(!a->m_captured)return;if(a->m_firstMouse){a->m_lastX=x;a->m_lastY=y;a->m_firstMouse=false;}a->m_camera.look(static_cast<float>(x-a->m_lastX),static_cast<float>(a->m_lastY-y));a->m_lastX=x;a->m_lastY=y;}
void Application::scroll(GLFWwindow*w,double,double y){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));a->m_camera.fov=std::clamp(a->m_camera.fov-static_cast<float>(y)*2.f,30.f,90.f);}
void Application::key(GLFWwindow*w,int k,int,int action,int){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));if(action!=GLFW_PRESS)return;if(k==GLFW_KEY_ESCAPE){a->m_captured=!a->m_captured;a->m_firstMouse=true;glfwSetInputMode(w,GLFW_CURSOR,a->m_captured?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL);}if(k==GLFW_KEY_F)a->m_flying=!a->m_flying;}
void Application::mouseButton(GLFWwindow*w,int button,int action,int){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));if(action!=GLFW_PRESS||!a->m_captured||a->m_clickCooldown>0)return;auto hit=Raycaster::cast(a->m_world,a->m_camera.position,a->m_camera.front());if(!hit.hit)return;if(button==GLFW_MOUSE_BUTTON_LEFT&&hit.block.y>0)a->m_world.setBlock(hit.block.x,hit.block.y,hit.block.z,BlockType::AIR);if(button==GLFW_MOUSE_BUTTON_RIGHT)a->m_world.setBlock(hit.adjacent.x,hit.adjacent.y,hit.adjacent.z,BlockType::DIRT);a->m_clickCooldown=.12f;}
void Application::moveWithCollision(glm::vec3 d){
  if(m_flying){m_camera.position+=d;return;}const glm::vec3 half{.3f,.9f,.3f};
  for(int axis=0;axis<3;++axis){glm::vec3 candidate=m_camera.position;candidate[axis]+=d[axis];bool blocked=false;glm::vec3 lo=candidate-half,hi=candidate+half;
    for(int y=static_cast<int>(std::floor(lo.y));y<=static_cast<int>(std::floor(hi.y));++y)for(int z=static_cast<int>(std::floor(lo.z));z<=static_cast<int>(std::floor(hi.z));++z)for(int x=static_cast<int>(std::floor(lo.x));x<=static_cast<int>(std::floor(hi.x));++x)if(isSolid(m_world.getBlock(x,y,z)))blocked=true;
    if(!blocked)m_camera.position=candidate;}
}
void Application::input(float dt){glm::vec3 f=m_camera.front();f.y=0;if(glm::length(f)>0)f=glm::normalize(f);glm::vec3 r=glm::normalize(glm::cross(f,glm::vec3{0,1,0}));glm::vec3 d{0};float speed=(glfwGetKey(m_window,GLFW_KEY_LEFT_CONTROL)==GLFW_PRESS?12.f:6.f)*dt;
  if(glfwGetKey(m_window,GLFW_KEY_W)==GLFW_PRESS)d+=f*speed;
  if(glfwGetKey(m_window,GLFW_KEY_S)==GLFW_PRESS)d-=f*speed;
  if(glfwGetKey(m_window,GLFW_KEY_D)==GLFW_PRESS)d+=r*speed;
  if(glfwGetKey(m_window,GLFW_KEY_A)==GLFW_PRESS)d-=r*speed;
  if(m_flying){if(glfwGetKey(m_window,GLFW_KEY_SPACE)==GLFW_PRESS)d.y+=speed;if(glfwGetKey(m_window,GLFW_KEY_LEFT_SHIFT)==GLFW_PRESS)d.y-=speed;}moveWithCollision(d);m_camera.position.x=std::clamp(m_camera.position.x,.31f,999.69f);m_camera.position.z=std::clamp(m_camera.position.z,.31f,999.69f);}
void Application::run(){double last=glfwGetTime();while(!glfwWindowShouldClose(m_window)){double now=glfwGetTime();float dt=std::min(static_cast<float>(now-last),.05f);last=now;m_clickCooldown=std::max(0.f,m_clickCooldown-dt);glfwPollEvents();input(dt);m_world.update(m_camera.position);
    int width,height;glfwGetFramebufferSize(m_window,&width,&height);glViewport(0,0,width,height);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glm::mat4 projection=glm::perspective(glm::radians(m_camera.fov),width/static_cast<float>(std::max(height,1)),.1f,500.f);m_renderer->draw(m_world,m_camera.view(),projection);glfwSwapBuffers(m_window);}}
