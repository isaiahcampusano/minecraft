#pragma once
#include "../renderer/Renderer.h"
#include "../world/World.h"
#include "../player/Player.h"
#include "../player/PlayerCamera.h"
#include "../player/Inventory.h"
#include "../player/SprintState.h"
#include "../player/EatingState.h"
#include "../world/LootTable.h"
#include "../world/PassiveMobSystem.h"
#include "DayNightCycle.h"
#include "SettingsState.h"
#include "WorldRepository.h"
#include "WorldSession.h"
#include "InputResolver.h"
#include "MenuController.h"
#include <GLFW/glfw3.h>
#include <memory>

class Application {
public:
  explicit Application(bool visible=true); ~Application(); void run();
private:
  friend struct CookingApplicationTests;
  friend struct MenusApplicationTests;
  enum class MenuState { Gameplay, Pause, Settings, Keybinds, Title, Worlds, Create, Delete, Swap };
  GLFWwindow* m_window=nullptr; WorldSession m_session; Player& m_player=m_session.player; PlayerCamera& m_camera=m_session.camera; World& m_world=m_session.world; Inventory& m_inventory=m_session.inventory; PassiveMobSystem& m_mobs=m_session.mobs; DayNightCycle& m_dayNight=m_session.dayNight; std::unique_ptr<Renderer> m_renderer;
  bool m_captured=true,m_firstMouse=true,m_inventoryOpen=false,m_tableOpen=false,m_inventoryDragging=false;
  bool m_furnaceOpen=false;glm::ivec3 m_furnacePosition{-1};int m_furnaceDragSource=-1;
  MenuState m_menu=MenuState::Title,m_settingsReturn=MenuState::Title,m_bindingsReturn=MenuState::Settings;
  WorldRepository m_repository;WorldInfo& m_activeWorld=m_session.info;bool& m_sessionActive=m_session.active;
  std::vector<WorldInfo> m_worlds;int m_selectedWorld=-1;
  MenuController m_menuController;InputResolver m_input;
  TextField m_worldName,m_seedText,m_chatEntry;int m_textTarget=0;
  bool m_chatReady=true,m_chatOpen=false,m_hudVisible=true,m_captureReady=false;GameMode m_newMode=GameMode::Survival;
  std::vector<std::string> m_messages,m_commandHistory;int m_historyIndex=0,m_chatScroll=0;
  std::string m_notice;int m_pendingAction=-1,m_pendingCode=-1;
  MenuView menuView()const;MenuView chatView()const;void activate(int id);void back();void bind(int action,int code);void slider(int id,float value);
  void refreshWorlds();bool enterWorld(const WorldInfo&);void unloadWorld();void applySettings();void routeInput(int code,int action,int mods=0);void gameplayPress(int code);void useAction();void command(const std::string&);void message(const std::string&);void syncCursor();
  static void character(GLFWwindow*,unsigned);static void focus(GLFWwindow*,int);static void closeWindow(GLFWwindow*);
  SettingsState m_settings;
  bool m_menuMouseReleaseRequired=false;
  int m_keybindCapture=-1;
  SprintState m_sprint; EatingState m_eating; LootTable m_loot; bool m_leftMouseWasDown=false,m_primaryAttackConsumed=false,m_miningAnimation=false;
  double m_lastX=0,m_lastY=0; float m_clickCooldown=0,m_attackCooldown=0,m_useSwingTimer=0; int m_creativePage=0;
  void setInventoryOpen(bool open,bool table=false);
  void setMenu(MenuState menu);
  void handleMenuClick(double x,double y,int width,int height);
  void input(float dt);
  void inventoryMouse(float mouseX,float mouseY,int width,int height,int action);
  void updateSimulation(float dt);
  void updateMining(float dt,const glm::vec3& origin,const glm::vec3& direction,const RayHit& blockHit);
  void updateEating(float dt);
  void respawnPlayer();
  bool saveGameState();
  void loadGameState();
  static void cursor(GLFWwindow*,double,double); static void scroll(GLFWwindow*,double,double); static void mouseButton(GLFWwindow*,int,int,int); static void key(GLFWwindow*,int,int,int,int);
};
