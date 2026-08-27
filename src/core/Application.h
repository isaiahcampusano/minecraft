#pragma once
#include "../renderer/Renderer.h"
#include "../world/World.h"
#include "../player/Player.h"
#include "../player/PlayerCamera.h"
#include "../player/Inventory.h"
#include "DayNightCycle.h"
#include <GLFW/glfw3.h>
#include <memory>

class Application {
public:
  Application(); ~Application(); void run();
private:
  GLFWwindow* m_window=nullptr; Player m_player; PlayerCamera m_camera; World m_world; Inventory m_inventory; DayNightCycle m_dayNight; std::unique_ptr<Renderer> m_renderer;
  bool m_captured=true,m_firstMouse=true,m_inventoryOpen=false,m_tableOpen=false; double m_lastX=0,m_lastY=0; float m_clickCooldown=0; int m_creativePage=0;
  void setInventoryOpen(bool open,bool table=false);
  void input(float dt);
  void updateMining(float dt);
  void saveGameState()const;
  void loadGameState();
  static void cursor(GLFWwindow*,double,double); static void scroll(GLFWwindow*,double,double); static void mouseButton(GLFWwindow*,int,int,int); static void key(GLFWwindow*,int,int,int,int);
};
