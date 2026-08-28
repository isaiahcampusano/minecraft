#pragma once
#include "../core/Shader.h"
#include "../world/Block.h"
#include <array>
#include <glad/gl.h>
#include <glm/glm.hpp>

class Texture;

class ParticleSystem {
public:
  ParticleSystem();
  ~ParticleSystem();
  ParticleSystem(const ParticleSystem&) = delete;
  ParticleSystem& operator=(const ParticleSystem&) = delete;

  void spawnBlockBreak(const glm::ivec3& block, BlockType type);
  void update(float deltaTime);
  void render(const glm::mat4& view,const glm::mat4& projection,const Texture& atlas,float daylight);
  std::size_t activeCount()const;

private:
  struct Particle {
    glm::vec3 position{0},velocity{0},axis{0,1,0};
    glm::vec4 uvRegion{0};
    float age=0,lifetime=0,angle=0,angularVelocity=0,size=.12f;
    bool active=false;
  };

  static constexpr std::size_t MAX_PARTICLES=256;
  std::array<Particle,MAX_PARTICLES> m_particles{};
  Shader m_shader;
  GLuint m_vao=0,m_vbo=0;
  unsigned m_randomState=0x6d2b79f5u;

  float random01();
  Particle& acquire();
};
