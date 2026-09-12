#include "Application.h"
#include "GameState.h"
#include "SaveLoad.h"
#include "../utils/Raycaster.h"
#include "../player/InventoryLayout.h"
#include "../player/CreativeCatalog.h"
#include "../player/Respawn.h"
#include "../world/BlockRegistry.h"
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
  SettingsState::load(m_settings);m_world.setViewDistance(m_settings.viewDistance);glfwSetInputMode(m_window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);glfwSwapInterval(1);glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glCullFace(GL_BACK);loadGameState();
}
Application::~Application(){saveGameState();SettingsState::save(m_settings);m_renderer.reset();if(m_window)glfwDestroyWindow(m_window);glfwTerminate();}
void Application::saveGameState()const{SaveLoad::save(GameState::capture(m_inventory,m_world,m_player,m_mobs));}
void Application::loadGameState(){SaveData data;if(SaveLoad::load(data))GameState::apply(data,m_inventory,m_world,m_player,m_mobs);}
void Application::setInventoryOpen(bool open,bool table){m_inventoryOpen=open;m_tableOpen=open&&table;m_inventoryDragging=false;m_miningAnimation=false;m_useSwingTimer=0;m_captured=!open;m_firstMouse=true;if(open){m_sprint.cancel();m_eating.cancel();}glfwSetInputMode(m_window,GLFW_CURSOR,m_captured?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL);}
void Application::setMenu(MenuState menu){if(menu==m_menu)return;m_menu=menu;m_keybindCapture=-1;m_menuMouseReleaseRequired=true;m_inventoryDragging=false;m_miningAnimation=false;m_useSwingTimer=0;m_sprint.cancel();m_eating.cancel();m_player.resetMiningProgress();m_player.resetMiningCooldown();m_captured=menu==MenuState::Gameplay;glfwSetInputMode(m_window,GLFW_CURSOR,m_captured?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL);}
void Application::handleMenuClick(double x,double y,int width,int height){
  if(m_menuMouseReleaseRequired)return;
  (void)x;(void)width;
  const float panelH=m_menu==MenuState::Settings?360.f:260.f,bottom=(height-panelH)*.5f;
  const float fromBottom=height-static_cast<float>(y)-bottom;
  if(m_menu==MenuState::Pause){
    if(fromBottom>panelH-130.f&&fromBottom<panelH-75.f)setMenu(MenuState::Gameplay);
    else if(fromBottom>panelH-180.f&&fromBottom<=panelH-130.f)setMenu(MenuState::Settings);
    else if(fromBottom>panelH-230.f&&fromBottom<=panelH-180.f){if(SaveLoad::save(GameState::capture(m_inventory,m_world,m_player,m_mobs)))glfwSetWindowShouldClose(m_window,GLFW_TRUE);}
  }else if(m_menu==MenuState::Settings){
    if(fromBottom>panelH-130.f&&fromBottom<panelH-75.f){m_player.setGameMode(m_player.gameMode()==GameMode::Survival?GameMode::Creative:GameMode::Survival);saveGameState();}
    else if(fromBottom>panelH-180.f&&fromBottom<=panelH-130.f){m_world.setViewDistance(m_world.viewDistance()>=8?2:m_world.viewDistance()+1);m_settings.setViewDistance(m_world.viewDistance());SettingsState::save(m_settings);}
    else if(fromBottom>panelH-230.f&&fromBottom<=panelH-180.f)setMenu(MenuState::Keybinds);
    else if(fromBottom<=panelH-230.f)setMenu(MenuState::Pause);
  }else if(m_menu==MenuState::Keybinds){
    if(fromBottom>panelH-190.f&&fromBottom<=panelH-130.f){m_settings.reset();SettingsState::save(m_settings);}
    else if(fromBottom>panelH-130.f&&fromBottom<panelH-75.f)m_keybindCapture=0;
    else if(fromBottom<=panelH-190.f)setMenu(MenuState::Settings);
  }
}
void Application::cursor(GLFWwindow*w,double x,double y){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));if(!a->m_captured)return;if(a->m_firstMouse){a->m_lastX=x;a->m_lastY=y;a->m_firstMouse=false;}a->m_player.look(static_cast<float>(x-a->m_lastX),static_cast<float>(a->m_lastY-y));a->m_lastX=x;a->m_lastY=y;}
void Application::scroll(GLFWwindow*w,double,double y){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));if(a->m_inventoryOpen){if(a->m_player.gameMode()!=GameMode::Creative)return;const int pages=static_cast<int>((creativeCatalog().size()+InventoryLayout::CREATIVE_PAGE_SIZE-1)/InventoryLayout::CREATIVE_PAGE_SIZE);a->m_creativePage=std::clamp(a->m_creativePage+(y<0?1:-1),0,std::max(0,pages-1));return;}a->m_camera.adjustFov(-static_cast<float>(y)*2.f);}
void Application::key(GLFWwindow*w,int k,int,int action,int){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));if(action!=GLFW_PRESS)return;
  if(a->m_keybindCapture>=0){if(k==GLFW_KEY_ESCAPE){a->m_keybindCapture=-1;return;}bool duplicate=false;for(std::size_t i=0;i<a->m_settings.keybindings.size();++i)if(static_cast<int>(i)!=a->m_keybindCapture&&a->m_settings.keybindings[i]==k)duplicate=true;if(!duplicate){a->m_settings.keybindings[static_cast<std::size_t>(a->m_keybindCapture)]=k;SettingsState::save(a->m_settings);a->m_keybindCapture=-1;}return;}
  if(k==GLFW_KEY_ESCAPE){if(a->m_inventoryOpen){a->setInventoryOpen(false);return;}if(a->m_menu==MenuState::Gameplay)a->setMenu(MenuState::Pause);else if(a->m_menu==MenuState::Keybinds)a->setMenu(MenuState::Settings);else if(a->m_menu==MenuState::Settings)a->setMenu(MenuState::Pause);else a->setMenu(MenuState::Gameplay);return;}
  if(a->m_menu!=MenuState::Gameplay)return;
  for(int i=0;i<9;++i){if(k==a->m_settings.key(static_cast<KeyAction>(static_cast<std::size_t>(KeyAction::Hotbar1)+i))){a->m_inventory.select(i);a->m_eating.cancel();return;}}
  if(k==a->m_settings.key(KeyAction::Inventory))a->setInventoryOpen(!a->m_inventoryOpen);else if(k==a->m_settings.key(KeyAction::ToggleFly)&&a->m_player.gameMode()==GameMode::Creative){a->m_player.toggleFly();if(a->m_player.isFlying){a->m_sprint.cancel();a->m_eating.cancel();}}else if(k==a->m_settings.key(KeyAction::Sprint)&&!a->m_inventoryOpen&&!a->m_player.isFlying&&a->m_player.onGround){a->m_eating.cancel();a->m_sprint.toggle();}else if(k==a->m_settings.key(KeyAction::MoveForward)&&!a->m_inventoryOpen&&!a->m_player.isFlying&&a->m_player.onGround){if(a->m_sprint.forwardPressed(glfwGetTime()))a->m_eating.cancel();}else if(k==a->m_settings.key(KeyAction::TogglePerspective))a->m_camera.togglePOV();}
void Application::mouseButton(GLFWwindow*w,int button,int action,int){
  auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));
  if(a->m_menu!=MenuState::Gameplay&&!a->m_inventoryOpen){if(action==GLFW_RELEASE)a->m_menuMouseReleaseRequired=false;if(action==GLFW_PRESS&&button==GLFW_MOUSE_BUTTON_LEFT){double x,y;int ww,hh;glfwGetCursorPos(w,&x,&y);glfwGetFramebufferSize(w,&ww,&hh);a->handleMenuClick(x,y,ww,hh);}return;}
  if(a->m_inventoryOpen){
    if(button!=GLFW_MOUSE_BUTTON_LEFT)return;
    double cursorX,cursorY;int windowWidth,windowHeight,frameWidth,frameHeight;
    glfwGetCursorPos(w,&cursorX,&cursorY);glfwGetWindowSize(w,&windowWidth,&windowHeight);glfwGetFramebufferSize(w,&frameWidth,&frameHeight);
    if(windowWidth<=0||windowHeight<=0)return;
    const auto hit=InventoryLayout::hitTest(static_cast<float>(cursorX*frameWidth/windowWidth),static_cast<float>(cursorY*frameHeight/windowHeight),frameWidth,frameHeight,a->m_tableOpen,a->m_player.gameMode()==GameMode::Creative);
    if(action==GLFW_RELEASE){
      if(!a->m_inventoryDragging)return;
      a->m_inventoryDragging=false;
      if(hit.area==InventoryLayout::Area::HOTBAR)a->m_inventory.swapHotbar(hit.index);
      else if(hit.area==InventoryLayout::Area::BACKPACK)a->m_inventory.swapBackpack(hit.index);
      else if(hit.area==InventoryLayout::Area::PERSONAL_CRAFT)a->m_inventory.swapCraft(false,hit.index);
      else if(hit.area==InventoryLayout::Area::TABLE_CRAFT)a->m_inventory.swapCraft(true,hit.index);
      return;
    }
    if(action!=GLFW_PRESS)return;
    if(hit.area==InventoryLayout::Area::HOTBAR){a->m_inventory.swapHotbar(hit.index);a->m_inventoryDragging=true;}
    else if(hit.area==InventoryLayout::Area::BACKPACK){a->m_inventory.swapBackpack(hit.index);a->m_inventoryDragging=true;}
    else if(hit.area==InventoryLayout::Area::PERSONAL_CRAFT){a->m_inventory.swapCraft(false,hit.index);a->m_inventoryDragging=true;}
    else if(hit.area==InventoryLayout::Area::TABLE_CRAFT){a->m_inventory.swapCraft(true,hit.index);a->m_inventoryDragging=true;}
    else if(hit.area==InventoryLayout::Area::CRAFT_OUTPUT){a->m_inventory.craftOutput(a->m_tableOpen);a->m_inventoryDragging=true;}
    else if(hit.area==InventoryLayout::Area::CREATIVE&&a->m_player.gameMode()==GameMode::Creative){const int itemIndex=a->m_creativePage*InventoryLayout::CREATIVE_PAGE_SIZE+hit.index;if(itemIndex<static_cast<int>(creativeCatalog().size())){a->m_inventory.giveCreative(creativeCatalog()[static_cast<std::size_t>(itemIndex)]);a->m_inventoryDragging=true;}}
    return;
  }
  if(action!=GLFW_PRESS||button!=GLFW_MOUSE_BUTTON_RIGHT||!a->m_captured||a->m_menu!=MenuState::Gameplay)return;
  if(a->m_clickCooldown>0)return;
  glm::vec3 origin=a->m_camera.position(a->m_player,a->m_world),direction=a->m_camera.direction(a->m_player,origin);auto hit=Raycaster::cast(a->m_world,origin,direction,a->m_camera.thirdPerson?12.f:6.f);if(!hit.hit){const ItemStack selected=a->m_inventory.selectedStack();if(selected.kind==ItemKind::FOOD&&!selected.empty()&&a->m_player.survival.canEat()){a->m_eating.start(a->m_inventory.selectedSlot(),selected.foodType);a->m_sprint.cancel();a->m_useSwingTimer=.2f;}return;}const BlockType targeted=a->m_world.getBlock(hit.block.x,hit.block.y,hit.block.z);if(targeted==BlockType::BED){a->m_useSwingTimer=.2f;if(a->m_dayNight.isNight()){const glm::vec3 bedSpawn=safeRespawnPosition(a->m_world,{hit.block.x+.5f,hit.block.y+1.f,hit.block.z+.5f});a->m_player.setSpawnPosition(bedSpawn);a->m_dayNight.skipToMorning();a->m_eating.cancel();a->m_sprint.cancel();}return;}if(targeted==BlockType::CRAFTING_TABLE){a->m_useSwingTimer=.2f;a->setInventoryOpen(true,true);return;}  const ItemStack selected=a->m_inventory.selectedStack();if(selected.kind==ItemKind::FOOD&&!selected.empty()){if(a->m_player.gameMode()==GameMode::Survival&&a->m_player.survival.canEat()){a->m_eating.start(a->m_inventory.selectedSlot(),selected.foodType);a->m_sprint.cancel();a->m_useSwingTimer=.2f;}return;}if(a->m_player.overlapsBlock(hit.adjacent))return;BlockType type=a->m_inventory.selectedType();if(type!=BlockType::AIR&&(a->m_player.gameMode()==GameMode::Creative||a->m_inventory.consumeSelected())){a->m_useSwingTimer=.2f;a->m_world.setBlock(hit.adjacent.x,hit.adjacent.y,hit.adjacent.z,type);}a->m_clickCooldown=.12f;
}
void Application::input(float){if(m_inventoryOpen||m_menu!=MenuState::Gameplay){m_player.velocity.x=0;m_player.velocity.z=0;if(m_player.isFlying)m_player.velocity.y=0;m_sprint.cancel();return;}glm::vec3 forward=m_player.forward();forward.y=0;if(glm::length(forward)>0)forward=glm::normalize(forward);glm::vec3 right=glm::normalize(glm::cross(forward,glm::vec3{0,1,0}));glm::vec3 move{0};
  if(glfwGetKey(m_window,m_settings.key(KeyAction::MoveForward))==GLFW_PRESS)move+=forward;
  if(glfwGetKey(m_window,m_settings.key(KeyAction::MoveBackward))==GLFW_PRESS)move-=forward;
  if(glfwGetKey(m_window,m_settings.key(KeyAction::MoveRight))==GLFW_PRESS)move+=right;
  if(glfwGetKey(m_window,m_settings.key(KeyAction::MoveLeft))==GLFW_PRESS)move-=right;
  if(glm::length(move)>0)move=glm::normalize(move);
  if(m_player.gameMode()==GameMode::Survival&&!m_player.survival.canSprint())m_sprint.cancel();
  const bool forwardIntent=glm::length(move)>0&&glm::dot(move,forward)>.1f;const bool canSprint=m_player.gameMode()==GameMode::Creative||m_player.survival.canSprint();m_sprint.evaluate(forwardIntent,!m_player.isFlying&&!m_eating.active()&&canSprint);const float speed=m_sprint.movementSpeed();m_player.velocity.x=move.x*speed;m_player.velocity.z=move.z*speed;
  if(m_player.isFlying){m_player.velocity.y=0;if(glfwGetKey(m_window,m_settings.key(KeyAction::Jump))==GLFW_PRESS)m_player.velocity.y=speed;if(glfwGetKey(m_window,m_settings.key(KeyAction::FlyDown))==GLFW_PRESS)m_player.velocity.y=-speed;}else if(glfwGetKey(m_window,m_settings.key(KeyAction::Jump))==GLFW_PRESS&&m_player.jump(m_sprint.active()))m_eating.cancel();
}
void Application::updateEating(float dt){if(!m_eating.active())return;EatingContext context;context.rightHeld=glfwGetMouseButton(m_window,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS;context.captured=m_captured;context.inventoryOpen=m_inventoryOpen;context.onGround=m_player.onGround;context.flying=m_player.isFlying;context.sprinting=m_sprint.active();context.canEat=m_player.survival.canEat();context.horizontalSpeed=glm::length(glm::vec2{m_player.velocity.x,m_player.velocity.z});context.selectedSlot=m_inventory.selectedSlot();context.selected=m_inventory.selectedStack();const FoodType food=m_eating.food();if(m_eating.update(dt,m_eating.canContinue(context))&&m_inventory.consumeSelectedFood(food))m_player.survival.eat(food);}
void Application::respawnPlayer(){m_sprint.cancel();m_eating.cancel();m_inventory.clear();const auto spawn=m_player.spawnPosition();m_world.loadChunk(static_cast<int>(spawn.x/16.f),static_cast<int>(spawn.z/16.f));m_player.respawn(safeRespawnPosition(m_world,spawn));}
void Application::updateMining(float dt,const glm::vec3& origin,const glm::vec3& direction,const RayHit& blockHit){const bool leftDown=glfwGetMouseButton(m_window,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;const bool justPressed=leftDown&&!m_leftMouseWasDown;m_leftMouseWasDown=leftDown;m_miningAnimation=false;if(!leftDown){m_primaryAttackConsumed=false;m_player.resetMiningProgress();m_player.resetMiningCooldown();return;}if(m_menu!=MenuState::Gameplay||m_inventoryOpen)return;if(justPressed&&!m_inventoryOpen&&m_captured){const MobHit mobHit=m_mobs.raycast(origin,direction);if(mobHit.hit&&(!blockHit.hit||mobHit.distance<blockHit.distance)){m_primaryAttackConsumed=true;m_player.resetMiningProgress();if(m_attackCooldown<=0){const ItemStack held=m_inventory.selectedStack();const bool damaged=m_mobs.damage(mobHit.id,PassiveMobSystem::attackDamage(held),m_player.position,m_loot,[this](const glm::vec3& position,const ItemStack& stack){m_renderer->spawnDrop(position,stack);});if(damaged){if(held.kind==ItemKind::TOOL&&m_player.gameMode()==GameMode::Survival)m_inventory.damageSelectedTool();m_attackCooldown=.35f;m_eating.cancel();}}return;}}if(m_primaryAttackConsumed){m_player.resetMiningProgress();return;}if(!m_captured||m_player.targetedBlock.y<=0){m_player.resetMiningProgress();return;}BlockType type=m_world.getBlock(m_player.targetedBlock.x,m_player.targetedBlock.y,m_player.targetedBlock.z);if(type==BlockType::AIR){m_player.clearMiningTarget();return;}m_miningAnimation=true;const ItemStack held=m_inventory.selectedStack();const auto& properties=getBlockProperties(type);if(m_player.gameMode()==GameMode::Creative)m_player.blockBreakProgress=1.f;if(m_player.advanceMining(type,dt,held)){const glm::ivec3 brokenBlock=m_player.targetedBlock;if(m_world.setBlock(brokenBlock.x,brokenBlock.y,brokenBlock.z,BlockType::AIR)){m_renderer->spawnBlockBreak(brokenBlock,type);if(m_player.gameMode()==GameMode::Survival&&canDropBlock(type,held))m_renderer->spawnDrop(glm::vec3(brokenBlock),ItemStack::block(miningDrop(type)));if(m_player.gameMode()==GameMode::Survival&&m_loot.dropsApple(type))m_renderer->spawnDrop(glm::vec3(brokenBlock),ItemStack::food(FoodType::APPLE));if(m_player.gameMode()==GameMode::Survival&&toolMatches(held,properties))m_inventory.damageSelectedTool();m_player.startMiningCooldown();}m_player.clearMiningTarget();}}
void Application::run(){double last=glfwGetTime();while(!glfwWindowShouldClose(m_window)){double now=glfwGetTime();float dt=std::min(static_cast<float>(now-last),.05f);last=now;m_clickCooldown=std::max(0.f,m_clickCooldown-dt);m_attackCooldown=std::max(0.f,m_attackCooldown-dt);m_useSwingTimer=std::max(0.f,m_useSwingTimer-dt);glfwPollEvents();const bool gameplay=m_menu==MenuState::Gameplay&&!m_inventoryOpen;if(gameplay)m_dayNight.update(dt);m_world.update(m_player.position);input(dt);const glm::vec3 previousPosition=m_player.position;const int previousHealth=m_player.survival.health();m_player.update(dt,m_world);if(gameplay&&m_sprint.active()&&m_player.gameMode()==GameMode::Survival)m_player.survival.addSprintDisplacement(m_player.position.x-previousPosition.x,m_player.position.z-previousPosition.z);if(gameplay&&m_player.gameMode()==GameMode::Survival)m_player.survival.update(dt);if(m_player.survival.health()<previousHealth)m_eating.cancel();else if(gameplay)updateEating(dt);if(m_player.survival.dead())respawnPlayer();if(gameplay)m_mobs.update(dt,m_world,m_player.position,m_dayNight.daylight());if(gameplay)m_renderer->updateDrops(dt,m_world,m_player.position,[this](const ItemStack& stack){return m_inventory.add(stack);});m_camera.updateFov(dt,m_sprint.active());
    int width,height;glfwGetFramebufferSize(m_window,&width,&height);glViewport(0,0,width,height);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glm::mat4 projection=glm::perspective(glm::radians(m_camera.fov),width/static_cast<float>(std::max(height,1)),.1f,500.f);glm::vec3 cameraPos=m_camera.position(m_player,m_world),cameraDir=m_camera.direction(m_player,cameraPos);RayHit target=Raycaster::cast(m_world,cameraPos,cameraDir,m_camera.thirdPerson?12.f:6.f);
    if(target.hit&&target.block.y>0)m_player.setMiningTarget(target.block);else m_player.clearMiningTarget();updateMining(dt,cameraPos,cameraDir,target);
    double cursorX=0,cursorY=0;int windowWidth=0,windowHeight=0;glfwGetCursorPos(m_window,&cursorX,&cursorY);glfwGetWindowSize(m_window,&windowWidth,&windowHeight);const float mouseX=windowWidth>0?static_cast<float>(cursorX*width/windowWidth):0.f,mouseY=windowHeight>0?static_cast<float>(cursorY*height/windowHeight):0.f;
    BlockType selected=target.hit?m_world.getBlock(target.block.x,target.block.y,target.block.z):BlockType::AIR;const char* name=selected==BlockType::AIR?"NONE":blockName(selected);
    std::ostringstream hud;hud<<std::fixed<<std::setprecision(1)<<"MODE: "<<(m_player.gameMode()==GameMode::Creative?"CREATIVE":"SURVIVAL")<<"\nPOS: "<<m_player.position.x<<" "<<m_player.position.y<<" "<<m_player.position.z<<"\nFLY: "<<(m_player.isFlying?"ON":"OFF")<<"\nSPRINT: "<<m_sprint.hudText()<<"\nTARGET: "<<name<<"\nTIME: "<<static_cast<int>(m_dayNight.time()*24.f)<<":00";
    m_renderer->draw(m_world,m_player,m_mobs,m_camera.thirdPerson,dt,m_miningAnimation,m_useSwingTimer,m_camera.view(m_player,cameraPos),projection,cameraPos,target,width,height,hud.str(),m_inventory,m_inventoryOpen,m_tableOpen,m_creativePage,mouseX,mouseY,m_dayNight,static_cast<int>(m_menu),m_world.viewDistance());glfwSwapBuffers(m_window);}}
