#include "core/Application.h"
#include "core/SaveLoad.h"
#include "player/InventoryLayout.h"
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#define REQUIRE(x) do { if(!(x))throw std::runtime_error(#x); } while(false)
struct CookingApplicationTests {
  static void click(Application& a,int slot){
    const auto r=FurnaceLayout::slots(1280)[slot];
    a.inventoryMouse(r.x+24,720-r.y-24,1280,720,GLFW_PRESS);
    a.inventoryMouse(r.x+24,720-r.y-24,1280,720,GLFW_RELEASE);
  }
  static void exercise(){
    {
      Application a(false);WorldInfo info;std::string error;REQUIRE(a.m_repository.create("Cooking",0,GameMode::Survival,info,error));REQUIRE(a.enterWorld(info));a.m_world.setTaskBudgets(0,0,0);a.m_player.position={500,7,500};a.m_player.yaw=-90;a.m_player.pitch=-15;
      a.m_player.setGameMode(GameMode::Creative);a.m_player.toggleFly();
      REQUIRE(a.m_world.setBlock(500,7,497,BlockType::FURNACE));
      Application::mouseButton(a.m_window,GLFW_MOUSE_BUTTON_RIGHT,GLFW_PRESS,0);
      REQUIRE(a.m_furnaceOpen&&a.m_inventoryOpen);
      a.m_inventory.setCursorStack(ItemStack::food(FoodType::RAW_BEEF,4));click(a,0);
      REQUIRE(a.m_inventory.cursorStack().empty()&&a.m_world.furnaceAt({500,7,497})->inputSlot.count==4);
      a.m_inventory.setCursorStack(ItemStack::block(BlockType::PLANKS,2));click(a,1);
      REQUIRE(a.m_inventory.cursorStack().empty());a.updateSimulation(2.f);
      REQUIRE(a.m_world.furnaceAt({500,7,497})->cookProgress==2.);
      // Remapping inventory must still close the furnace without returning its contents.
      a.m_settings.key(KeyAction::Inventory)=GLFW_KEY_I;
      Application::key(a.m_window,GLFW_KEY_I,0,GLFW_PRESS,0);REQUIRE(!a.m_inventoryOpen&&!a.m_furnaceOpen);
      a.updateSimulation(1.f);REQUIRE(a.m_world.furnaceAt({500,7,497})->cookProgress==3.);
      const auto time=a.m_dayNight.time(),position=a.m_player.position.y;
      Application::key(a.m_window,GLFW_KEY_ESCAPE,0,GLFW_PRESS,0);a.updateSimulation(20.f);
      REQUIRE(a.m_world.furnaceAt({500,7,497})->cookProgress==3.&&a.m_dayNight.time()==time&&a.m_player.position.y==position);
      a.setMenu(Application::MenuState::Settings);a.updateSimulation(20.f);REQUIRE(a.m_world.furnaceAt({500,7,497})->cookProgress==3.);
      a.setMenu(Application::MenuState::Gameplay);a.updateSimulation(7.f);
      REQUIRE(a.m_world.furnaceAt({500,7,497})->outputSlot.count==1);
      a.m_menuMouseReleaseRequired=false;Application::mouseButton(a.m_window,GLFW_MOUSE_BUTTON_RIGHT,GLFW_RELEASE,0);Application::mouseButton(a.m_window,GLFW_MOUSE_BUTTON_RIGHT,GLFW_PRESS,0);
      REQUIRE(a.m_furnaceOpen);click(a,2);REQUIRE(a.m_inventory.cursorStack().foodType==FoodType::COOKED_BEEF&&a.m_inventory.cursorStack().count==1);
      Application::key(a.m_window,GLFW_KEY_ESCAPE,0,GLFW_PRESS,0);REQUIRE(a.m_inventory.cursorStack().count==1);
      a.saveGameState();
    }
    {
      Application a(false);a.loadGameState();a.m_world.setTaskBudgets(0,0,0);a.m_world.loadChunk(31,31);
      const auto* f=a.m_world.furnaceAt({500,7,497});REQUIRE(f&&f->fuelRemaining==5.&&f->inputSlot.count==3&&f->outputSlot.empty());
      REQUIRE(a.m_inventory.cursorStack().foodType==FoodType::COOKED_BEEF&&a.m_settings.key(KeyAction::Inventory)==GLFW_KEY_I);
      a.m_player.setGameMode(GameMode::Survival);a.m_player.survival.restore(20,5,0,0);a.m_world.tickFurnaces(10.);
      ItemStack cursor;REQUIRE(a.m_world.interactFurnace({500,7,497},FurnaceSlot::Output,cursor));REQUIRE(a.m_player.survival.eat(cursor.foodType)&&a.m_player.survival.hunger()==13);
      a.setInventoryOpen(true);a.m_furnaceOpen=true;a.m_furnacePosition={500,7,497};a.m_world.setBlock(500,7,497,BlockType::AIR);a.updateSimulation(0.f);REQUIRE(!a.m_inventoryOpen);
    }
  }
};
int main(){
  if(!glfwInit())return 77;
  glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
  auto* probe=glfwCreateWindow(64,64,"Cooking test probe",nullptr,nullptr);
  if(!probe){glfwTerminate();return 77;}glfwDestroyWindow(probe);glfwTerminate();
  const auto root=std::filesystem::temp_directory_path()/"minecraft-cooking-application-tests";
  std::filesystem::create_directories(root);
#ifdef _WIN32
  _putenv_s("APPDATA",root.string().c_str());
#else
  setenv("HOME",root.string().c_str(),1);
#endif
  int result=0;
  try{CookingApplicationTests::exercise();}catch(const std::exception& e){std::cerr<<e.what()<<'\n';result=1;}
  std::filesystem::remove_all(root);return result;
}
