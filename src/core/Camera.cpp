#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

glm::vec3 Camera::front() const {
  glm::vec3 f;
  f.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
  f.y = std::sin(glm::radians(pitch));
  f.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
  return glm::normalize(f);
}
glm::vec3 Camera::right() const { return glm::normalize(glm::cross(front(), {0,1,0})); }
glm::mat4 Camera::view() const { return glm::lookAt(position, position + front(), {0,1,0}); }
void Camera::look(float dx, float dy) { yaw += dx * 0.1f; pitch = std::clamp(pitch + dy * 0.1f, -89.0f, 89.0f); }

