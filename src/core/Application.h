#pragma once
#include "../renderer/Renderer.h"
#include "../world/World.h"
#include "../player/Player.h"
#include "../player/PlayerCamera.h"
#include "../player/Inventory.h"
#include <GLFW/glfw3.h>
#include <memory>

class Application {
public:
  Application(); ~Application(); void run();
private:
  GLFWwindow* m_window=nullptr; Player m_player; PlayerCamera m_camera; World m_world; Inventory m_inventory; std::unique_ptr<Renderer> m_renderer;
  bool m_captured=true,m_firstMouse=true; double m_lastX=0,m_lastY=0; float m_clickCooldown=0;
  void input(float dt);
  static void cursor(GLFWwindow*,double,double); static void scroll(GLFWwindow*,double,double); static void mouseButton(GLFWwindow*,int,int,int); static void key(GLFWwindow*,int,int,int,int);
};
