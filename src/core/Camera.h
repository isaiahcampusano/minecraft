#pragma once
#include <glm/glm.hpp>

class Camera {
public:
  glm::vec3 position{500.0f, 7.0f, 500.0f};
  float yaw = -90.0f, pitch = -20.0f, fov = 70.0f;
  glm::vec3 front() const;
  glm::vec3 right() const;
  glm::mat4 view() const;
  void look(float dx, float dy);
};

