#pragma once
#include <glm/vec3.hpp>

class DayNightCycle {
public:
  static constexpr float DAY_LENGTH_SECONDS = 720.f;
  static constexpr float NIGHT_START = .50f;
  static constexpr float MORNING_TIME = .25f;
  void update(float dt);
  float time() const { return m_time; }
  bool isNight() const;
  void skipToMorning() { m_time = MORNING_TIME; }
  float daylight() const;
  glm::vec3 sunDirection() const;
  glm::vec3 skyTop() const;
  glm::vec3 skyBottom() const;
private:
  float m_time = 0.25f;
};
