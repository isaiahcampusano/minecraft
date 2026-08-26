#pragma once
#include "Camera.h"
#include "../renderer/Renderer.h"
#include "../world/World.h"
#include <GLFW/glfw3.h>
#include <memory>

class Application {
public:
  Application(); ~Application(); void run();
private:
  GLFWwindow* m_window=nullptr; Camera m_camera; World m_world; std::unique_ptr<Renderer> m_renderer;
  bool m_captured=true,m_flying=false,m_firstMouse=true; double m_lastX=0,m_lastY=0; float m_clickCooldown=0;
  void input(float dt); void moveWithCollision(glm::vec3 delta);
  bool blockOverlapsPlayer(const glm::ivec3& block) const;
  static void cursor(GLFWwindow*,double,double); static void scroll(GLFWwindow*,double,double); static void mouseButton(GLFWwindow*,int,int,int); static void key(GLFWwindow*,int,int,int,int);
};
