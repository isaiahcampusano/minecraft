#include "Shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <vector>

GLuint Shader::compile(GLenum type, const char* source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  GLint ok = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
  if (!ok) {
    GLint length = 0; glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    std::vector<char> log(static_cast<std::size_t>(length));
    glGetShaderInfoLog(shader, length, nullptr, log.data());
    glDeleteShader(shader);
    throw std::runtime_error(std::string("Shader compilation failed: ") + log.data());
  }
  return shader;
}

Shader::Shader(const char* vs, const char* fs) {
  GLuint vertex = compile(GL_VERTEX_SHADER, vs), fragment = compile(GL_FRAGMENT_SHADER, fs);
  m_program = glCreateProgram();
  glAttachShader(m_program, vertex); glAttachShader(m_program, fragment); glLinkProgram(m_program);
  glDeleteShader(vertex); glDeleteShader(fragment);
  GLint ok = GL_FALSE; glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
  if (!ok) { glDeleteProgram(m_program); throw std::runtime_error("Shader link failed"); }
}
Shader::~Shader() { if (m_program) glDeleteProgram(m_program); }
void Shader::use() const { glUseProgram(m_program); }
void Shader::setMat4(const char* n, const glm::mat4& v) const { glUniformMatrix4fv(glGetUniformLocation(m_program, n), 1, GL_FALSE, glm::value_ptr(v)); }
void Shader::setInt(const char* n, int v) const { glUniform1i(glGetUniformLocation(m_program, n), v); }

