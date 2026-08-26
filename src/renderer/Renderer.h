#pragma once
#include "Texture.h"
#include "../core/Shader.h"
#include <glm/mat4x4.hpp>
class World;
class Renderer { public: Renderer(); void draw(const World&,const glm::mat4&,const glm::mat4&); private: Shader m_shader; Texture m_texture; };

