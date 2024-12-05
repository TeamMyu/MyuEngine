#pragma once

#include <glad/glad.h>

class VertexArrayObject
{
public:
    VertexArrayObject();
    ~VertexArrayObject();

    void bind() const;
    void unbind() const;

    void setAttributePointer(GLuint index, GLint size, GLenum type,
                           GLboolean normalized, GLsizei stride, const void* pointer);
    void enableAttribute(GLuint index);

private:
    GLuint id;
};