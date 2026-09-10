#include "Renderer.h"
#include "../utils/Raycaster.h"
#include "../world/World.h"
#include "PlayerRenderer.h"
#include "MobRenderer.h"
#include "../player/Player.h"
#include "../player/Inventory.h"
#include "../player/InventoryLayout.h"
#include "../player/CraftingLayout.h"
#include "../player/MaterialRegistry.h"
#include "../player/ToolRegistry.h"
#include "../player/FoodRegistry.h"
#include "../player/SurvivalHud.h"
#include "../player/CreativeCatalog.h"
#include "../world/BlockRegistry.h"
#include "../core/DayNightCycle.h"
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include <algorithm>
#include <vector>

namespace {
const char* VS=R"(#version 330 core
layout(location=0)in vec3 p;layout(location=1)in vec2 uv;layout(location=2)in float shade;
out vec2 vUV;out float vShade;out vec3 vWorld;uniform mat4 projection,view;
void main(){vUV=uv;vShade=shade;vWorld=p;gl_Position=projection*view*vec4(p,1.0);})";
const char* FS=R"(#version 330 core
in vec2 vUV;in float vShade;in vec3 vWorld;out vec4 color;uniform sampler2D atlas;uniform vec3 cameraPos;uniform vec3 fogColor;uniform float daylight;
void main(){vec4 base=texture(atlas,vUV)*vec4(vec3(vShade),1.0);vec3 lit=base.rgb*mix(0.15,1.0,daylight);float d=length(vWorld-cameraPos);float fog=1.0-exp(-0.00055*d*d);color=vec4(mix(lit,fogColor,clamp(fog,0.0,0.92)),base.a);})";
const char* COLOR_VS=R"(#version 330 core
layout(location=0)in vec3 p;uniform mat4 transform;void main(){gl_Position=transform*vec4(p,1.0);})";
const char* COLOR_FS=R"(#version 330 core
out vec4 color;uniform vec4 tint;void main(){color=tint;})";
const char* SKY_VS=R"(#version 330 core
layout(location=0)in vec2 p;out float height;void main(){height=p.y;gl_Position=vec4(p,0.999,1.0);})";
const char* SKY_FS=R"(#version 330 core
in float height;out vec4 color;uniform vec3 topColor,bottomColor;void main(){float t=height*.5+.5;color=vec4(mix(bottomColor,topColor,t),1.0);})";

std::array<unsigned char,7> glyph(char c){
  switch(c){
    case 'A':return{14,17,17,31,17,17,17};case 'B':return{30,17,17,30,17,17,30};case 'C':return{14,17,16,16,16,17,14};
    case 'D':return{30,17,17,17,17,17,30};case 'E':return{31,16,16,30,16,16,31};case 'F':return{31,16,16,30,16,16,16};
    case 'G':return{14,17,16,23,17,17,14};case 'H':return{17,17,17,31,17,17,17};case 'I':return{14,4,4,4,4,4,14};case 'K':return{17,18,20,24,20,18,17};
    case 'L':return{16,16,16,16,16,16,31};case 'M':return{17,27,21,21,17,17,17};case 'N':return{17,25,21,19,17,17,17};case 'O':return{14,17,17,17,17,17,14};
    case 'P':return{30,17,17,30,16,16,16};case 'R':return{30,17,17,30,20,18,17};case 'S':return{15,16,16,14,1,1,30};
    case 'T':return{31,4,4,4,4,4,4};case 'U':return{17,17,17,17,17,17,14};case 'V':return{17,17,17,17,17,10,4};case 'W':return{17,17,17,21,21,21,10};case 'X':return{17,17,10,4,10,17,17};case 'Y':return{17,17,10,4,4,4,4};case '0':return{14,17,19,21,25,17,14};
    case '1':return{4,12,4,4,4,4,14};case '2':return{14,17,1,2,4,8,31};case '3':return{30,1,1,14,1,1,30};
    case '4':return{2,6,10,18,31,2,2};case '5':return{31,16,16,30,1,1,30};case '6':return{14,16,16,30,17,17,14};
    case '7':return{31,1,2,4,8,8,8};case '8':return{14,17,17,14,17,17,14};case '9':return{14,17,17,15,1,1,14};
    case ':':return{0,4,4,0,4,4,0};case '.':return{0,0,0,0,0,4,4};case '-':return{0,0,0,31,0,0,0};default:return{};
  }
}
}

Renderer::Renderer():m_shader(VS,FS),m_colorShader(COLOR_VS,COLOR_FS),m_skyShader(SKY_VS,SKY_FS){
  glGenVertexArrays(1,&m_lineVao);glGenBuffers(1,&m_lineVbo);glBindVertexArray(m_lineVao);glBindBuffer(GL_ARRAY_BUFFER,m_lineVbo);glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),nullptr);glEnableVertexAttribArray(0);
  const float sky[]={-1,-1,1,-1,-1,1,1,1};glGenVertexArrays(1,&m_skyVao);glGenBuffers(1,&m_skyVbo);glBindVertexArray(m_skyVao);glBindBuffer(GL_ARRAY_BUFFER,m_skyVbo);glBufferData(GL_ARRAY_BUFFER,sizeof(sky),sky,GL_STATIC_DRAW);glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),nullptr);glEnableVertexAttribArray(0);glBindVertexArray(0);m_playerRenderer=std::make_unique<PlayerRenderer>();m_mobRenderer=std::make_unique<MobRenderer>();
}
Renderer::~Renderer(){m_mobRenderer.reset();m_playerRenderer.reset();if(m_skyVbo)glDeleteBuffers(1,&m_skyVbo);if(m_skyVao)glDeleteVertexArrays(1,&m_skyVao);if(m_lineVbo)glDeleteBuffers(1,&m_lineVbo);if(m_lineVao)glDeleteVertexArrays(1,&m_lineVao);}
void Renderer::drawSky(const DayNightCycle& dayNight){glDisable(GL_DEPTH_TEST);m_skyShader.use();m_skyShader.setVec3("topColor",dayNight.skyTop());m_skyShader.setVec3("bottomColor",dayNight.skyBottom());glBindVertexArray(m_skyVao);glDrawArrays(GL_TRIANGLE_STRIP,0,4);glEnable(GL_DEPTH_TEST);}
void Renderer::drawOutline(const RayHit& hit,const glm::mat4& view,const glm::mat4& projection){if(!hit.hit)return;constexpr float e=.002f;float x=hit.block.x-e,y=hit.block.y-e,z=hit.block.z-e,s=1.f+2*e;
  const float p[]={x,y,z,x+s,y,z, x+s,y,z,x+s,y+s,z, x+s,y+s,z,x,y+s,z, x,y+s,z,x,y,z,
    x,y,z+s,x+s,y,z+s, x+s,y,z+s,x+s,y+s,z+s, x+s,y+s,z+s,x,y+s,z+s, x,y+s,z+s,x,y,z+s,
    x,y,z,x,y,z+s, x+s,y,z,x+s,y,z+s, x+s,y+s,z,x+s,y+s,z+s, x,y+s,z,x,y+s,z+s};
  glBindBuffer(GL_ARRAY_BUFFER,m_lineVbo);glBufferData(GL_ARRAY_BUFFER,sizeof(p),p,GL_DYNAMIC_DRAW);m_colorShader.use();m_colorShader.setMat4("transform",projection*view);m_colorShader.setVec4("tint",{.35f,.95f,1.f,1});glLineWidth(3.f);glBindVertexArray(m_lineVao);glDrawArrays(GL_LINES,0,24);
}
void Renderer::drawMiningCrack(const glm::ivec3& block,float progress,const glm::mat4& view,const glm::mat4& projection){if(progress<=0.f||block.y<0)return;int stage=std::min(9,static_cast<int>(progress*10.f));float e=.006f,x=block.x-e,y=block.y-e,z=block.z-e,s=1.f+2*e;const float p[]={x,y,z,x+s,y,z,x+s,y,z,x+s,y+s,z,x+s,y+s,z,x,y+s,z,x,y+s,z,x,y,z,x,y,z+s,x+s,y,z+s,x+s,y,z+s,x+s,y+s,z+s,x+s,y+s,z+s,x,y+s,z+s,x,y+s,z+s,x,y,z+s,x,y,z,x,y,z+s,x+s,y,z,x+s,y,z+s,x+s,y+s,z,x+s,y+s,z+s,x,y+s,z,x,y+s,z+s};glBindBuffer(GL_ARRAY_BUFFER,m_lineVbo);glBufferData(GL_ARRAY_BUFFER,sizeof(p),p,GL_DYNAMIC_DRAW);m_colorShader.use();m_colorShader.setMat4("transform",projection*view);m_colorShader.setVec4("tint",{1.f,1.f,1.f,.2f+stage*.08f});glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);glLineWidth(3.f+stage*.7f);glBindVertexArray(m_lineVao);glDrawArrays(GL_LINES,0,24);glLineWidth(1.f);glDisable(GL_BLEND);}
void Renderer::drawOverlay(int width,int height,const std::string& text,const Player& player,const Inventory& inventory,bool inventoryOpen,bool tableOpen,int creativePage,float mouseX,float mouseY){
  const glm::mat4 screen=glm::ortho(0.f,static_cast<float>(width),0.f,static_cast<float>(height));
  auto drawQuads=[&](const std::vector<float>& vertices,const glm::vec4& color){if(vertices.empty())return;glBindBuffer(GL_ARRAY_BUFFER,m_lineVbo);glBufferData(GL_ARRAY_BUFFER,static_cast<GLsizeiptr>(vertices.size()*sizeof(float)),vertices.data(),GL_DYNAMIC_DRAW);m_colorShader.use();m_colorShader.setMat4("transform",screen);m_colorShader.setVec4("tint",color);glBindVertexArray(m_lineVao);glDrawArrays(GL_TRIANGLES,0,static_cast<GLsizei>(vertices.size()/3));};
  auto appendQuad=[](std::vector<float>& vertices,float x,float y,float w,float h){float q[]={x,y,0,x+w,y,0,x+w,y+h,0,x,y,0,x+w,y+h,0,x,y+h,0};vertices.insert(vertices.end(),q,q+18);};
  auto drawText=[&](const std::string& value,float x,float y,float pixel,const glm::vec4& color){std::vector<float> pixels;float penX=x,penY=y;for(char c:value){if(c=='\n'){penX=x;penY-=9*pixel;continue;}auto rows=glyph(c);for(int row=0;row<7;++row)for(int col=0;col<5;++col)if(rows[row]&(1<<(4-col)))appendQuad(pixels,penX+col*pixel,penY-row*pixel,pixel,pixel);penX+=6*pixel;}drawQuads(pixels,color);};
  auto tintFor=[](BlockType type){const BlockColor color=blockColor(type);return glm::vec4{color.r/255.f,color.g/255.f,color.b/255.f,1.f};};
  auto itemTint=[&](const ItemStack& stack){if(stack.kind==ItemKind::BLOCK)return tintFor(stack.blockType);const ItemColor color=stack.kind==ItemKind::MATERIAL?materialColor(stack.materialType):stack.kind==ItemKind::FOOD?foodColor(stack.foodType):toolColor(stack.toolKind,stack.toolTier);return glm::vec4{color.r/255.f,color.g/255.f,color.b/255.f,1.f};};
  auto drawSlot=[&](float x,float y,const ItemStack& stack,bool selected){std::vector<float> border,background;if(selected)appendQuad(border,x-4,y-4,InventoryLayout::SLOT_SIZE+8,InventoryLayout::SLOT_SIZE+8);else appendQuad(border,x-2,y-2,InventoryLayout::SLOT_SIZE+4,InventoryLayout::SLOT_SIZE+4);appendQuad(background,x,y,InventoryLayout::SLOT_SIZE,InventoryLayout::SLOT_SIZE);drawQuads(border,selected?glm::vec4{.95f,.95f,.88f,1.f}:glm::vec4{.38f,.38f,.40f,.95f});drawQuads(background,{.16f,.16f,.18f,.94f});if(stack.empty())return;std::vector<float> swatch;appendQuad(swatch,x+8,y+8,InventoryLayout::SLOT_SIZE-16,InventoryLayout::SLOT_SIZE-16);drawQuads(swatch,itemTint(stack));if(stack.kind==ItemKind::TOOL){const float ratio=std::clamp(stack.durability/static_cast<float>(std::max(1,maxToolDurability(stack.toolTier))),0.f,1.f);std::vector<float> bar,fill;appendQuad(bar,x+5,y+4,InventoryLayout::SLOT_SIZE-10,4);appendQuad(fill,x+5,y+4,(InventoryLayout::SLOT_SIZE-10)*ratio,4);drawQuads(bar,{.08f,.08f,.08f,1.f});drawQuads(fill,{1.f-ratio,ratio,.08f,1.f});}else{const std::string count=std::to_string(stack.count);drawText(count,x+InventoryLayout::SLOT_SIZE-4.f-static_cast<float>(count.size())*6.f,y+10.f,1.f,{1,1,1,1});}};

  glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  std::vector<float> hud;float cx=width*.5f,cy=height*.5f;appendQuad(hud,cx-10,cy-2,20,4);appendQuad(hud,cx-2,cy-10,4,20);drawQuads(hud,{.95f,.95f,.95f,.95f});drawText(text,12.f,height-18.f,2.f,{.05f,.05f,.05f,.9f});

  const float startX=InventoryLayout::hotbarX(width);
  if(!inventoryOpen){const std::array<unsigned char,7> heart={27,63,127,127,62,28,8},hunger={8,28,30,15,7,14,12};auto drawMeterIcon=[&](const std::array<unsigned char,7>&shape,float x,float y,int value,const glm::vec4&color){std::vector<float>empty,filled;for(int row=0;row<7;++row)for(int col=0;col<7;++col)if(shape[static_cast<std::size_t>(row)]&(1<<(6-col))){appendQuad(empty,x+col*2.f,y+(6-row)*2.f,2.f,2.f);if(value>=2||(value==1&&col<4))appendQuad(filled,x+col*2.f,y+(6-row)*2.f,2.f,2.f);}drawQuads(empty,{.18f,.18f,.18f,.95f});drawQuads(filled,color);};const float meterY=InventoryLayout::HOTBAR_Y+InventoryLayout::SLOT_SIZE+10.f;for(int i=0;i<10;++i){const int healthValue=survivalIconValue(player.survival.health(),i),hungerValue=survivalIconValue(player.survival.hunger(),i);drawMeterIcon(heart,startX+i*16.f,meterY,healthValue,{.82f,.08f,.12f,1.f});drawMeterIcon(hunger,startX+InventoryLayout::hotbarWidth()-160.f+i*16.f,meterY,hungerValue,{.78f,.43f,.12f,1.f});}}
  if(inventoryOpen){
    const float paletteX=InventoryLayout::paletteX(width),panelTop=InventoryLayout::BACKPACK_Y+InventoryLayout::CREATIVE_PAGE_SIZE*(InventoryLayout::PALETTE_HEIGHT+InventoryLayout::GAP)+20.f;
    std::vector<float> panel;appendQuad(panel,paletteX-8,InventoryLayout::BACKPACK_Y-10,InventoryLayout::PALETTE_WIDTH+16,panelTop-InventoryLayout::BACKPACK_Y+18);appendQuad(panel,startX-8,InventoryLayout::BACKPACK_Y-10,InventoryLayout::hotbarWidth()+16,CraftingLayout::BOTTOM-InventoryLayout::BACKPACK_Y+3*(InventoryLayout::SLOT_SIZE+InventoryLayout::GAP)+24);drawQuads(panel,{.08f,.08f,.09f,.98f});
    for(int i=0;i<Inventory::BACKPACK_SLOTS;++i){const int row=i/9,col=i%9;drawSlot(startX+col*(InventoryLayout::SLOT_SIZE+InventoryLayout::GAP),InventoryLayout::BACKPACK_Y+row*(InventoryLayout::SLOT_SIZE+InventoryLayout::GAP),inventory.backpackSlot(i),false);}
    for(int i=0;i<InventoryLayout::CREATIVE_PAGE_SIZE;++i){const int itemIndex=creativePage*InventoryLayout::CREATIVE_PAGE_SIZE+i;if(itemIndex>=static_cast<int>(creativeCatalog().size()))break;const ItemStack item=creativeCatalog()[static_cast<std::size_t>(itemIndex)];const float entryY=InventoryLayout::BACKPACK_Y+i*(InventoryLayout::PALETTE_HEIGHT+InventoryLayout::GAP);std::vector<float> edge,entry,swatch;appendQuad(edge,paletteX-2,entryY-2,InventoryLayout::PALETTE_WIDTH+4,InventoryLayout::PALETTE_HEIGHT+4);appendQuad(entry,paletteX,entryY,InventoryLayout::PALETTE_WIDTH,InventoryLayout::PALETTE_HEIGHT);appendQuad(swatch,paletteX+5,entryY+4,24,24);drawQuads(edge,{.38f,.38f,.40f,1.f});drawQuads(entry,{.16f,.16f,.18f,1.f});drawQuads(swatch,itemTint(item));drawText(itemName(item),paletteX+35,entryY+20,1.f,{1,1,1,1});}
    drawText("ITEMS",paletteX,panelTop,1.f,{1,1,1,1});drawText("PAGE "+std::to_string(creativePage+1),paletteX+78,panelTop,1.f,{1,1,1,1});
    const int craftSize=tableOpen?3:2;for(int i=0;i<craftSize*craftSize;++i){const int row=i/craftSize,col=i%craftSize;drawSlot(CraftingLayout::gridX(width,tableOpen)+col*(InventoryLayout::SLOT_SIZE+InventoryLayout::GAP),CraftingLayout::BOTTOM+(craftSize-1-row)*(InventoryLayout::SLOT_SIZE+InventoryLayout::GAP),inventory.craftSlot(tableOpen,i),false);}drawSlot(CraftingLayout::outputX(width),CraftingLayout::outputY(tableOpen),inventory.craftingOutput(tableOpen),false);drawText(tableOpen?"TABLE CRAFT":"PERSONAL CRAFT",CraftingLayout::gridX(width,tableOpen),CraftingLayout::BOTTOM+craftSize*(InventoryLayout::SLOT_SIZE+InventoryLayout::GAP)+8.f,1.f,{1,1,1,1});drawText("OUTPUT",CraftingLayout::outputX(width),CraftingLayout::outputY(tableOpen)+62.f,1.f,{1,1,1,1});
    const float heldX=startX+InventoryLayout::hotbarWidth()+16.f;drawSlot(heldX,InventoryLayout::BACKPACK_Y,inventory.cursorStack(),false);drawText("HELD",heldX,InventoryLayout::BACKPACK_Y+62.f,1.f,{1,1,1,1});
  }
  for(int i=0;i<Inventory::HOTBAR_SLOTS;++i)drawSlot(startX+i*(InventoryLayout::SLOT_SIZE+InventoryLayout::GAP),InventoryLayout::HOTBAR_Y,inventory.hotbarSlot(i),i==inventory.selectedSlot());
  auto hoveredStack=[&](){const auto hit=InventoryLayout::hitTest(mouseX,mouseY,width,height,tableOpen);if(hit.area==InventoryLayout::Area::HOTBAR){if(!inventoryOpen&&hit.index>6)return ItemStack{};return inventory.hotbarSlot(hit.index);}if(!inventoryOpen)return ItemStack{};if(hit.area==InventoryLayout::Area::BACKPACK)return inventory.backpackSlot(hit.index);if(hit.area==InventoryLayout::Area::PERSONAL_CRAFT)return inventory.craftSlot(false,hit.index);if(hit.area==InventoryLayout::Area::TABLE_CRAFT)return inventory.craftSlot(true,hit.index);if(hit.area==InventoryLayout::Area::CRAFT_OUTPUT)return inventory.craftingOutput(tableOpen);if(hit.area==InventoryLayout::Area::HELD)return inventory.cursorStack();if(hit.area==InventoryLayout::Area::CREATIVE){const int itemIndex=creativePage*InventoryLayout::CREATIVE_PAGE_SIZE+hit.index;if(itemIndex>=0&&itemIndex<static_cast<int>(creativeCatalog().size()))return creativeCatalog()[static_cast<std::size_t>(itemIndex)];}return ItemStack{};};
  const ItemStack hovered=hoveredStack();
  if(!hovered.empty()){
    const std::string label=itemName(hovered);const float pixel=2.f,boxW=static_cast<float>(label.size())*6.f*pixel+12.f,boxH=26.f;
    float x=std::clamp(mouseX+14.f,4.f,std::max(4.f,static_cast<float>(width)-boxW-4.f));float y=std::clamp(static_cast<float>(height)-mouseY-24.f,4.f,std::max(4.f,static_cast<float>(height)-boxH-4.f));
    std::vector<float> tooltip;appendQuad(tooltip,x,y,boxW,boxH);drawQuads(tooltip,{.04f,.04f,.05f,.92f});drawText(label,x+6.f,y+19.f,pixel,{1,1,1,1});
  }
  glDisable(GL_BLEND);glEnable(GL_CULL_FACE);glEnable(GL_DEPTH_TEST);
}
void Renderer::draw(const World&w,const Player&player,const PassiveMobSystem&mobs,bool showPlayer,float dt,const glm::mat4&v,const glm::mat4&p,const glm::vec3&camera,const RayHit&hit,int width,int height,const std::string&hud,const Inventory&inventory,bool inventoryOpen,bool tableOpen,int creativePage,float mouseX,float mouseY,const DayNightCycle&dayNight){drawSky(dayNight);m_shader.use();m_shader.setMat4("view",v);m_shader.setMat4("projection",p);m_shader.setInt("atlas",0);m_shader.setVec3("cameraPos",camera);m_shader.setVec3("fogColor",dayNight.skyBottom());m_shader.setFloat("daylight",dayNight.daylight());m_texture.bind();w.render();if(showPlayer)m_playerRenderer->draw(player,dt,v,p,camera,dayNight);else{glDisable(GL_DEPTH_TEST);glDisable(GL_CULL_FACE);m_playerRenderer->drawFirstPerson(player,v,p,camera,dayNight);glEnable(GL_CULL_FACE);glEnable(GL_DEPTH_TEST);}m_mobRenderer->draw(mobs,w,v,p,camera,dayNight);m_particles.update(dt);m_particles.render(v,p,m_texture,dayNight.daylight());m_drops.render(v,p,m_texture,dayNight.daylight());drawOutline(hit,v,p);drawMiningCrack(player.targetedBlock,player.blockBreakProgress,v,p);drawOverlay(width,height,hud,player,inventory,inventoryOpen,tableOpen,creativePage,mouseX,mouseY);}
