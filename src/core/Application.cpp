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

#include "../player/PlayerPhysics.h"
Application::Application(bool visible){
  if(!glfwInit())throw std::runtime_error("Could not initialize GLFW");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);glfwWindowHint(GLFW_VISIBLE,visible?GLFW_TRUE:GLFW_FALSE);
  m_window=glfwCreateWindow(1280,720,"Minecraft Superflat",nullptr,nullptr);if(!m_window){glfwTerminate();throw std::runtime_error("Could not create window");}
  glfwMakeContextCurrent(m_window);if(!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)))throw std::runtime_error("Could not load OpenGL");
  m_renderer=std::make_unique<Renderer>();glfwSetWindowUserPointer(m_window,this);glfwSetCursorPosCallback(m_window,cursor);glfwSetScrollCallback(m_window,scroll);glfwSetMouseButtonCallback(m_window,mouseButton);glfwSetKeyCallback(m_window,key);glfwSetCharCallback(m_window,character);glfwSetWindowFocusCallback(m_window,focus);glfwSetWindowCloseCallback(m_window,closeWindow);
  SettingsState::load(m_settings);m_notice=m_settings.notice;std::string imported;m_repository.importLegacy(imported);if(!imported.empty()){if(!m_notice.empty())m_notice+=". ";m_notice+=imported;}applySettings();syncCursor();glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);glCullFace(GL_BACK);
}
Application::~Application(){SettingsState::save(m_settings);m_world.reset();m_renderer.reset();if(m_window)glfwDestroyWindow(m_window);glfwTerminate();}
bool Application::saveGameState(){if(!m_sessionActive)return true;auto data=GameState::capture(m_inventory,m_world,m_player,m_mobs);data.timeOfDay=m_dayNight.time();return m_repository.save(m_activeWorld,data,m_notice);}
void Application::loadGameState(){refreshWorlds();if(!m_worlds.empty())enterWorld(m_worlds.front());}
void Application::syncCursor(){m_captured=m_sessionActive&&m_menu==MenuState::Gameplay&&!m_inventoryOpen&&!m_chatOpen;m_firstMouse=true;m_input.suppress();glfwSetInputMode(m_window,GLFW_CURSOR,m_captured?GLFW_CURSOR_DISABLED:GLFW_CURSOR_NORMAL);if(glfwRawMouseMotionSupported())glfwSetInputMode(m_window,GLFW_RAW_MOUSE_MOTION,m_captured&&m_settings.rawMouseInput);}
void Application::setInventoryOpen(bool open,bool table){m_furnaceOpen=false;m_furnaceDragSource=-1;m_menuMouseReleaseRequired=true;m_player.resetMiningProgress();m_inventoryOpen=open;m_tableOpen=open&&table;m_inventoryDragging=false;m_miningAnimation=false;m_useSwingTimer=0;m_sprint.cancel();m_eating.cancel();syncCursor();}
void Application::setMenu(MenuState menu){m_menu=menu;m_keybindCapture=-1;m_menuController.reset();m_textTarget=0;m_menuMouseReleaseRequired=true;m_inventoryDragging=false;m_miningAnimation=false;m_useSwingTimer=0;m_sprint.cancel();m_eating.cancel();m_player.resetMiningProgress();m_player.resetMiningCooldown();syncCursor();auto view=menuView();if(!view.widgets.empty()&&!view.widgets.front().enabled)m_menuController.navigate(view,1);}
void Application::cursor(GLFWwindow*w,double x,double y){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));if(!a->m_captured){int ww,hh;glfwGetWindowSize(w,&ww,&hh);if(ww>0&&hh>0){float px=static_cast<float>(x*1280/ww),py=static_cast<float>(y*720/hh);auto view=a->menuView();a->m_menuController.hover=a->m_menuController.hit(view,px,py);int pressed=a->m_menuController.pressed;if(pressed>=0&&pressed<static_cast<int>(view.widgets.size())&&view.widgets[pressed].value>=0){auto& item=view.widgets[pressed];a->slider(item.id,(px-item.x)/item.w);}}return;}if(a->m_firstMouse){a->m_lastX=x;a->m_lastY=y;a->m_firstMouse=false;return;}const float sensitivity=a->m_settings.mouseSensitivity;a->m_player.look(static_cast<float>(x-a->m_lastX)*sensitivity,static_cast<float>(a->m_lastY-y)*sensitivity*(a->m_settings.invertY?-1.f:1.f));a->m_lastX=x;a->m_lastY=y;}
void Application::scroll(GLFWwindow*w,double,double y){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));if(a->m_chatOpen){int lines=0;for(const auto& message:a->m_messages)lines+=static_cast<int>((message.size()+99)/100);a->m_chatScroll=std::clamp(a->m_chatScroll+(y<0?-1:1),0,std::max(0,lines-12));return;}if(a->m_menu!=MenuState::Gameplay){int count=a->m_menu==MenuState::Worlds?static_cast<int>(a->m_worlds.size()):static_cast<int>(SettingsState::ACTION_COUNT);a->m_menuController.scroll=std::clamp(a->m_menuController.scroll+(y<0?1:-1),0,std::max(0,count-8));return;}if(a->m_inventoryOpen){if(a->m_player.gameMode()!=GameMode::Creative)return;int pages=static_cast<int>((creativeCatalog().size()+InventoryLayout::CREATIVE_PAGE_SIZE-1)/InventoryLayout::CREATIVE_PAGE_SIZE);a->m_creativePage=std::clamp(a->m_creativePage+(y<0?1:-1),0,std::max(0,pages-1));return;}if(a->m_captured)a->m_inventory.select((a->m_inventory.selectedSlot()+(y<0?1:8))%9);}
void Application::key(GLFWwindow*w,int code,int,int action,int mods){static_cast<Application*>(glfwGetWindowUserPointer(w))->routeInput(code,action,mods);}
void Application::mouseButton(GLFWwindow*w,int button,int action,int mods){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));a->routeInput(SettingsState::MOUSE_OFFSET+button,action,mods);}
void Application::focus(GLFWwindow*w,int focused){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));if(!focused){a->m_input.suppress();a->m_chatOpen=false;if(a->m_inventoryOpen)a->setInventoryOpen(false);if(a->m_sessionActive)a->setMenu(MenuState::Pause);a->m_firstMouse=true;}else{for(int k=32;k<=GLFW_KEY_LAST;++k)if(SettingsState::validCode(k))a->m_input.event(k,glfwGetKey(w,k)==GLFW_PRESS);for(int b=0;b<=GLFW_MOUSE_BUTTON_LAST;++b)a->m_input.event(1000+b,glfwGetMouseButton(w,b)==GLFW_PRESS);a->m_input.suppress();}}
void Application::closeWindow(GLFWwindow*w){auto*a=static_cast<Application*>(glfwGetWindowUserPointer(w));if(!a->saveGameState()){glfwSetWindowShouldClose(w,GLFW_FALSE);a->setMenu(MenuState::Pause);}}
void Application::useAction(){auto*a=this;
  if(a->m_clickCooldown>0)return;
  glm::vec3 origin=a->m_player.eyePosition(),direction=a->m_player.forward();auto hit=Raycaster::cast(a->m_world,origin,direction,6.f);if(!hit.hit){const ItemStack selected=a->m_inventory.selectedStack();if(selected.kind==ItemKind::FOOD&&!selected.empty()&&a->m_player.gameMode()==GameMode::Survival&&a->m_player.survival.canEat()){a->m_eating.start(a->m_inventory.selectedSlot(),selected.foodType);a->m_sprint.cancel();a->m_useSwingTimer=.2f;}return;}const BlockType targeted=a->m_world.getBlock(hit.block.x,hit.block.y,hit.block.z);if(targeted==BlockType::FURNACE){a->setInventoryOpen(true);a->m_furnaceOpen=true;a->m_furnacePosition=hit.block;return;}if(targeted==BlockType::BED){a->m_useSwingTimer=.2f;if(a->m_dayNight.isNight()){const glm::vec3 bedSpawn=safeRespawnPosition(a->m_world,{hit.block.x+.5f,hit.block.y+1.f,hit.block.z+.5f});a->m_player.setSpawnPosition(bedSpawn);a->m_dayNight.skipToMorning();a->m_eating.cancel();a->m_sprint.cancel();}return;}if(targeted==BlockType::CRAFTING_TABLE){a->m_useSwingTimer=.2f;a->setInventoryOpen(true,true);return;}  const ItemStack selected=a->m_inventory.selectedStack();if(selected.kind==ItemKind::FOOD&&!selected.empty()){if(a->m_player.gameMode()==GameMode::Survival&&a->m_player.gameMode()==GameMode::Survival&&a->m_player.survival.canEat()){a->m_eating.start(a->m_inventory.selectedSlot(),selected.foodType);a->m_sprint.cancel();a->m_useSwingTimer=.2f;}return;}if(a->m_player.overlapsBlock(hit.adjacent))return;BlockType type=a->m_inventory.selectedType();if(type!=BlockType::AIR&&(a->m_player.gameMode()==GameMode::Creative||a->m_inventory.consumeSelected())){a->m_useSwingTimer=.2f;a->m_world.setBlock(hit.adjacent.x,hit.adjacent.y,hit.adjacent.z,type);}a->m_clickCooldown=.12f;
}
void Application::inventoryMouse(float mouseX,float mouseY,int width,int height,int action){
  auto* a=this;
    const auto hit=InventoryLayout::hitTest(mouseX,mouseY,width,height,a->m_tableOpen,a->m_player.gameMode()==GameMode::Creative,a->m_furnaceOpen);
    if(a->m_furnaceOpen){
      if(!a->m_world.furnaceAt(a->m_furnacePosition)){a->setInventoryOpen(false);return;}
      const int identity=static_cast<int>(hit.area)*100+hit.index;
      if(action==GLFW_RELEASE){const int source=a->m_furnaceDragSource;a->m_furnaceDragSource=-1;if(source<0||source==identity)return;}
      else if(action!=GLFW_PRESS)return;
      ItemStack cursor=a->m_inventory.cursorStack();bool changed=false;
      if(hit.area==InventoryLayout::Area::FURNACE)changed=a->m_world.interactFurnace(a->m_furnacePosition,static_cast<FurnaceSlot>(hit.index),cursor);
      else if(hit.area==InventoryLayout::Area::HOTBAR||hit.area==InventoryLayout::Area::BACKPACK){
        const bool hotbar=hit.area==InventoryLayout::Area::HOTBAR;
        ItemStack slot=hotbar?a->m_inventory.hotbarSlot(hit.index):a->m_inventory.backpackSlot(hit.index);
        if(!cursor.empty()&&!slot.empty()&&cursor.kind!=ItemKind::TOOL&&sameItemType(slot,cursor)){
          const int amount=std::min(cursor.count,64-slot.count);slot.count+=amount;cursor.count-=amount;if(cursor.count==0)cursor={};changed=amount>0;
        }else{std::swap(slot,cursor);changed=true;}
        if(hotbar)a->m_inventory.setHotbarSlot(hit.index,slot);else a->m_inventory.setBackpackSlot(hit.index,slot);
      }else if(hit.area==InventoryLayout::Area::CREATIVE&&a->m_player.gameMode()==GameMode::Creative&&action==GLFW_PRESS){
        const int index=a->m_creativePage*InventoryLayout::CREATIVE_PAGE_SIZE+hit.index;
        if(index<static_cast<int>(creativeCatalog().size())){a->m_inventory.giveCreative(creativeCatalog()[index]);cursor=a->m_inventory.cursorStack();changed=true;}
      }
      if(changed)a->m_inventory.setCursorStack(cursor);
      if(action==GLFW_PRESS)a->m_furnaceDragSource=changed?identity:-1;
      return;
    }
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
void Application::input(float){if(m_inventoryOpen||m_chatOpen||m_menu!=MenuState::Gameplay){m_player.velocity.x=0;m_player.velocity.z=0;if(m_player.isFlying)m_player.velocity.y=0;m_sprint.cancel();m_player.isSneaking=false;return;}glm::vec3 forward=m_player.forward();forward.y=0;if(glm::length(forward)>0)forward=glm::normalize(forward);glm::vec3 right=glm::normalize(glm::cross(forward,glm::vec3{0,1,0}));glm::vec3 move{0};
  if(m_input.held(m_settings,KeyAction::MoveForward,m_player.isFlying))move+=forward;
  if(m_input.held(m_settings,KeyAction::MoveBackward,m_player.isFlying))move-=forward;
  if(m_input.held(m_settings,KeyAction::MoveRight,m_player.isFlying))move+=right;
  if(m_input.held(m_settings,KeyAction::MoveLeft,m_player.isFlying))move-=right;
  if(glm::length(move)>0)move=glm::normalize(move);
  if(m_player.gameMode()==GameMode::Survival&&!m_player.survival.canSprint())m_sprint.cancel();
  const bool forwardIntent=glm::length(move)>0&&glm::dot(move,forward)>.1f;const bool canSprint=m_player.gameMode()==GameMode::Creative||m_player.survival.canSprint();m_sprint.evaluate(forwardIntent,!m_player.isFlying&&!m_eating.active()&&canSprint);m_player.isSneaking=m_input.held(m_settings,KeyAction::Sneak,m_player.isFlying);if(m_player.isSneaking)m_sprint.cancel();const float speed=m_sprint.movementSpeed()*(m_player.isSneaking?.3f:1.f);m_player.velocity.x=move.x*speed;m_player.velocity.z=move.z*speed;
  if(m_player.isFlying){m_player.velocity.y=0;if(m_input.held(m_settings,KeyAction::Jump,m_player.isFlying))m_player.velocity.y=speed;if(m_input.held(m_settings,KeyAction::FlyDown,m_player.isFlying))m_player.velocity.y=-speed;}else if(m_input.held(m_settings,KeyAction::Jump,m_player.isFlying)&&m_player.jump(m_sprint.active()))m_eating.cancel();
}
void Application::updateEating(float dt){if(m_player.gameMode()==GameMode::Creative){m_eating.cancel();return;}if(!m_eating.active())return;EatingContext context;context.rightHeld=m_input.held(m_settings,KeyAction::Use);context.captured=m_captured;context.inventoryOpen=m_inventoryOpen;context.onGround=m_player.onGround;context.flying=m_player.isFlying;context.sprinting=m_sprint.active();context.canEat=m_player.survival.canEat();context.horizontalSpeed=glm::length(glm::vec2{m_player.velocity.x,m_player.velocity.z});context.selectedSlot=m_inventory.selectedSlot();context.selected=m_inventory.selectedStack();const FoodType food=m_eating.food();if(m_eating.update(dt,m_eating.canContinue(context))&&m_inventory.consumeSelectedFood(food))m_player.survival.eat(food);}
void Application::respawnPlayer(){if(m_inventoryOpen)setInventoryOpen(false);m_sprint.cancel();m_eating.cancel();m_inventory.clear();const auto spawn=m_player.spawnPosition();m_world.loadChunk(static_cast<int>(spawn.x/16.f),static_cast<int>(spawn.z/16.f));m_player.respawn(safeRespawnPosition(m_world,spawn));}
void Application::updateMining(float dt,const glm::vec3& origin,const glm::vec3& direction,const RayHit& blockHit){const bool justPressed=m_input.justPressed(m_settings,KeyAction::Attack);const bool leftDown=m_input.held(m_settings,KeyAction::Attack)||justPressed;m_leftMouseWasDown=leftDown;m_miningAnimation=false;if(!leftDown){m_primaryAttackConsumed=false;m_player.resetMiningProgress();m_player.resetMiningCooldown();return;}if(m_menu!=MenuState::Gameplay||m_inventoryOpen||m_chatOpen||m_menuMouseReleaseRequired)return;if(justPressed&&!m_inventoryOpen&&m_captured){const MobHit mobHit=m_mobs.raycast(origin,direction);if(mobHit.hit&&(!blockHit.hit||mobHit.distance<blockHit.distance)){m_primaryAttackConsumed=true;m_player.resetMiningProgress();if(m_attackCooldown<=0){const ItemStack held=m_inventory.selectedStack();const bool damaged=m_mobs.damage(mobHit.id,PassiveMobSystem::attackDamage(held),m_player.position,m_loot,[this](const glm::vec3& position,const ItemStack& stack){m_renderer->spawnDrop(position,stack);});if(damaged){if(held.kind==ItemKind::TOOL&&m_player.gameMode()==GameMode::Survival)m_inventory.damageSelectedTool();m_attackCooldown=.35f;m_eating.cancel();}}return;}}if(m_primaryAttackConsumed){m_player.resetMiningProgress();return;}if(!m_captured||m_player.targetedBlock.y<=0){m_player.resetMiningProgress();return;}BlockType type=m_world.getBlock(m_player.targetedBlock.x,m_player.targetedBlock.y,m_player.targetedBlock.z);if(type==BlockType::AIR){m_player.clearMiningTarget();return;}m_miningAnimation=true;const ItemStack held=m_inventory.selectedStack();const auto& properties=getBlockProperties(type);if(m_player.gameMode()==GameMode::Creative)m_player.blockBreakProgress=1.f;if(m_player.advanceMining(type,dt,held)){const glm::ivec3 brokenBlock=m_player.targetedBlock;if(m_world.setBlock(brokenBlock.x,brokenBlock.y,brokenBlock.z,BlockType::AIR,[this](const glm::vec3& p,const ItemStack& stack){m_renderer->spawnDrop(p,stack);})){m_renderer->spawnBlockBreak(brokenBlock,type);if(m_player.gameMode()==GameMode::Survival&&canDropBlock(type,held))m_renderer->spawnDrop(glm::vec3(brokenBlock),ItemStack::block(miningDrop(type)));if(m_player.gameMode()==GameMode::Survival&&m_loot.dropsApple(type))m_renderer->spawnDrop(glm::vec3(brokenBlock),ItemStack::food(FoodType::APPLE));if(m_player.gameMode()==GameMode::Survival&&toolMatches(held,properties))m_inventory.damageSelectedTool();m_player.startMiningCooldown();}m_player.clearMiningTarget();}}
void Application::updateSimulation(float dt){
  if(!m_sessionActive)return;
  const bool gameplay=m_menu==MenuState::Gameplay;if(gameplay)m_dayNight.update(dt);m_world.update(m_player.position);if(gameplay)m_world.tickFurnaces(dt);if(m_furnaceOpen&&!m_world.furnaceAt(m_furnacePosition))setInventoryOpen(false);input(dt);const glm::vec3 previousPosition=m_player.position;const int previousHealth=m_player.survival.health();if(gameplay)m_player.update(dt,m_world);if(gameplay&&m_sprint.active()&&m_player.gameMode()==GameMode::Survival)m_player.survival.addSprintDisplacement(m_player.position.x-previousPosition.x,m_player.position.z-previousPosition.z);if(gameplay&&m_player.gameMode()==GameMode::Survival)m_player.survival.update(dt);if(m_player.survival.health()<previousHealth)m_eating.cancel();else if(gameplay)updateEating(dt);if(gameplay&&m_player.gameMode()==GameMode::Survival&&m_player.survival.dead())respawnPlayer();if(gameplay)m_mobs.update(dt,m_world,m_player.position,m_dayNight.daylight());if(gameplay)m_renderer->updateDrops(dt,m_world,m_player.position,[this](const ItemStack& stack){return m_inventory.add(stack);});m_camera.updateFov(dt,m_sprint.active(),m_captured&&m_input.held(m_settings,KeyAction::Zoom));
}

void Application::run(){double last=glfwGetTime();while(!glfwWindowShouldClose(m_window)){
  double now=glfwGetTime();float dt=std::min(static_cast<float>(now-last),.05f);last=now;
  m_clickCooldown=std::max(0.f,m_clickCooldown-dt);m_attackCooldown=std::max(0.f,m_attackCooldown-dt);m_useSwingTimer=std::max(0.f,m_useSwingTimer-dt);glfwPollEvents();m_menuMouseReleaseRequired=!m_input.allReleased()&&!m_captured;
  updateSimulation(dt);int width,height;glfwGetFramebufferSize(m_window,&width,&height);if(width<=0||height<=0){m_input.endFrame();continue;}glViewport(0,0,width,height);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  if(m_sessionActive){glm::mat4 projection=glm::perspective(glm::radians(m_camera.fov),width/static_cast<float>(height),.1f,500.f);glm::vec3 cameraPos=m_camera.position(m_player,m_world);RayHit target=Raycaster::cast(m_world,m_player.eyePosition(),m_player.forward(),6.f);
    if(target.hit&&target.block.y>0)m_player.setMiningTarget(target.block);else m_player.clearMiningTarget();updateMining(dt,m_player.eyePosition(),m_player.forward(),target);
    double x,y;int ww,hh;glfwGetCursorPos(m_window,&x,&y);glfwGetWindowSize(m_window,&ww,&hh);float mx=ww>0?static_cast<float>(x*width/ww):0,my=hh>0?static_cast<float>(y*height/hh):0;
    std::ostringstream hud;hud<<std::fixed<<std::setprecision(1)<<"MODE: "<<(m_player.gameMode()==GameMode::Creative?"CREATIVE":"SURVIVAL")<<"\nPOS: "<<m_player.position.x<<" "<<m_player.position.y<<" "<<m_player.position.z<<"\nSEED: "<<m_world.seed()<<"\n"<<m_notice;
    m_renderer->hudVisible=m_hudVisible;
    m_renderer->draw(m_world,m_player,m_mobs,m_camera.isThirdPerson(),m_menu==MenuState::Gameplay?dt:0.f,m_miningAnimation,m_useSwingTimer,m_camera.view(m_player,cameraPos),projection,cameraPos,target,width,height,hud.str(),m_inventory,m_inventoryOpen,m_tableOpen,m_creativePage,mx,my,m_dayNight,0,m_world.viewDistance(),m_furnaceOpen?m_world.furnaceAt(m_furnacePosition):nullptr);
    if(m_chatOpen){auto chat=chatView();MenuController control;control.focus=static_cast<int>(chat.widgets.size())-1;m_renderer->drawMenu(chat,control,width,height);}
  }
  if(m_menu!=MenuState::Gameplay)m_renderer->drawMenu(menuView(),m_menuController,width,height);
  glfwSwapBuffers(m_window);m_input.endFrame();
}}
