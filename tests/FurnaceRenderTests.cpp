#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "renderer/Renderer.h"
#include "world/World.h"
#include "world/PassiveMobSystem.h"
#include "player/Player.h"
#include "player/Inventory.h"
#include "core/DayNightCycle.h"
#include "utils/Raycaster.h"
#include <glm/gtc/matrix_transform.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc,char** argv){
  if(!glfwInit())return 77;
  glfwWindowHint(GLFW_VISIBLE,GLFW_FALSE);glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* window=glfwCreateWindow(1280,720,"Furnace render verification",nullptr,nullptr);
  if(!window){glfwTerminate();return 77;}glfwMakeContextCurrent(window);
  if(!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress))){glfwDestroyWindow(window);glfwTerminate();return 77;}
  int result=0;
  {
    Renderer renderer;World world;Player player;PassiveMobSystem mobs;Inventory inventory;DayNightCycle day;
    const glm::ivec3 position{500,7,497};world.setBlock(500,7,497,BlockType::FURNACE);world.setBlock(501,7,497,BlockType::FURNACE);
    ItemStack cursor=ItemStack::food(FoodType::RAW_BEEF,16);world.interactFurnace(position,FurnaceSlot::Input,cursor);
    cursor=ItemStack::block(BlockType::PLANKS,16);world.interactFurnace(position,FurnaceSlot::Fuel,cursor);world.tickFurnaces(14.);
    world.updateLighting();world.setTaskBudgets(0,20,20);world.update({500,8,500});
    inventory.setHotbarSlot(0,ItemStack::food(FoodType::COOKED_BEEF,4));inventory.setHotbarSlot(1,ItemStack::block(BlockType::PLANKS,16));
    glEnable(GL_DEPTH_TEST);glEnable(GL_CULL_FACE);
    for(int scale:{1,2}){
      const int width=1280*scale,height=720*scale;
      GLuint framebuffer,color,depth;glGenFramebuffers(1,&framebuffer);glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
      glGenRenderbuffers(1,&color);glBindRenderbuffer(GL_RENDERBUFFER,color);glRenderbufferStorage(GL_RENDERBUFFER,GL_RGB8,width,height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_RENDERBUFFER,color);
      glGenRenderbuffers(1,&depth);glBindRenderbuffer(GL_RENDERBUFFER,depth);glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,width,height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,depth);
      if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){std::cerr<<"Incomplete test framebuffer\n";result=1;}
      glReadBuffer(GL_COLOR_ATTACHMENT0);
      for(int screen=0;screen<3;++screen){
        const bool open=screen!=0;player.setGameMode(screen==2?GameMode::Creative:GameMode::Survival);
        const glm::vec3 camera{500.5f,9.8f,502.f};const auto view=glm::lookAt(camera,glm::vec3(500.5f,7.7f,497.f),glm::vec3(0,1,0));
        const auto projection=glm::perspective(glm::radians(70.f),width/static_cast<float>(height),.1f,500.f);
        glViewport(0,0,width,height);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        renderer.draw(world,player,mobs,false,0,false,0,view,projection,camera,RayHit{},width,height,"COOKING VERIFICATION",inventory,open,false,0,0,0,day,0,4,open?world.furnaceAt(position):nullptr);
        glFinish();std::vector<unsigned char> pixels(width*height*3);glPixelStorei(GL_PACK_ALIGNMENT,1);glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
        std::size_t orange=0;for(std::size_t i=0;i<pixels.size();i+=3)if(pixels[i]>240&&pixels[i+1]>65&&pixels[i+1]<120&&pixels[i+2]<40)++orange;
        if(glGetError()!=GL_NO_ERROR||orange==0){std::cerr<<"Furnace render missing flame or GL error\n";result=1;}
        if(argc>1){
          std::filesystem::create_directories(argv[1]);const auto path=std::filesystem::path(argv[1])/(std::string(screen==0?"world":screen==1?"survival":"creative")+"-"+std::to_string(scale)+".ppm");
          std::ofstream file(path,std::ios::binary);file<<"P6\n"<<width<<' '<<height<<"\n255\n";
          for(int y=height-1;y>=0;--y)file.write(reinterpret_cast<const char*>(pixels.data()+y*width*3),width*3);
        }
      }
      glBindFramebuffer(GL_FRAMEBUFFER,0);glDeleteRenderbuffers(1,&color);glDeleteRenderbuffers(1,&depth);glDeleteFramebuffers(1,&framebuffer);
    }
  }
  glfwDestroyWindow(window);glfwTerminate();return result;
}
