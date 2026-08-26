#pragma once
#include "../core/Shader.h"
#include <glad/gl.h>
#include <glm/mat4x4.hpp>
class Player;
class PlayerRenderer{public:PlayerRenderer();~PlayerRenderer();void draw(const Player&,const glm::mat4&,const glm::mat4&);private:Shader m_shader;GLuint m_vao=0,m_vbo=0;GLsizei m_count=0;};

