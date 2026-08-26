#pragma once
#include <glad/gl.h>
class Texture { public: Texture(); ~Texture(); void bind() const; private: GLuint m_id=0; };

