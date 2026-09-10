#pragma once
#include "../core/Shader.h"
#include "../player/Item.h"
#include <array>
#include <cstddef>
#include <functional>
#include <glad/gl.h>
#include <glm/glm.hpp>

class Texture;
class World;

class DropSystem {
public:
  using PickupHandler=std::function<bool(const ItemStack&)>;

  DropSystem();
  ~DropSystem();
  DropSystem(const DropSystem&)=delete;
  DropSystem& operator=(const DropSystem&)=delete;

  void spawn(const glm::vec3& position,const ItemStack& stack);
  void update(float dt,const World& world,const glm::vec3& playerPosition,const PickupHandler& tryPickup);
  void render(const glm::mat4& view,const glm::mat4& projection,const Texture& atlas,float daylight);
  std::size_t activeCount()const;

private:
  struct Drop {
    glm::vec3 position{0},velocity{0};
    ItemStack stack{};
    float age=0,spin=0,pickupDelay=.4f;
    bool onGround=false,active=false;
  };

  static constexpr std::size_t MAX_DROPS=128;
  static constexpr float HALF_SIZE=.125f;
  static constexpr float PICKUP_RADIUS=1.5f;
  static constexpr float BOB_HEIGHT=.06f;
  static constexpr float BOB_SPEED=2.2f;
  static constexpr float MAX_AGE=300.f;

  std::array<Drop,MAX_DROPS> m_drops{};
  Shader m_shader;
  GLuint m_vao=0,m_vbo=0;
  unsigned m_randomState=0x3c6ef372u;

  float random01();
  Drop& acquire();
};
