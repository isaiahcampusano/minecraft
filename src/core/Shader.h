#pragma once
#include <glad/gl.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <string>

class Shader {
public:
  Shader(const char* vertexSource, const char* fragmentSource);
  ~Shader();
  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;
  void use() const;
  void setMat4(const char* name, const glm::mat4& value) const;
  void setInt(const char* name, int value) const;
  void setFloat(const char* name, float value) const;
  void setVec3(const char* name, const glm::vec3& value) const;
  void setVec4(const char* name, const glm::vec4& value) const;
private:
  GLuint m_program = 0;
  static GLuint compile(GLenum type, const char* source);
};
