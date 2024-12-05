#pragma once

#include <vector>
#include <glad/glad.h>

class VertexBufferObject
{
public:
    VertexBufferObject();
    ~VertexBufferObject();

    void bind() const;
    void unbind() const;

    void write(const std::vector<float>& vertices);

    GLuint getID() const;

private:
    GLuint id;
};