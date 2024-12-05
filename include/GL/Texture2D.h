#pragma once

#include "glad/glad.h"

#include <string>

class Texture2D
{
public:
    Texture2D(const std::string& path, GLint format, GLenum fileFormat);
    ~Texture2D();

    void bind(GLenum unit);

private:
    GLuint id;
};