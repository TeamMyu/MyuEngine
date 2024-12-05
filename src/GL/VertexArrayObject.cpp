#include "GL/VertexArrayObject.h"

VertexArrayObject::VertexArrayObject()
{
    glGenVertexArrays(1, &id);
}

VertexArrayObject::~VertexArrayObject()
{
    glDeleteVertexArrays(1, &id);
}

void VertexArrayObject::bind() const
{
    glBindVertexArray(id);
}

void VertexArrayObject::unbind() const
{
    glBindVertexArray(0);
}

void VertexArrayObject::setAttributePointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer)
{
    glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

void VertexArrayObject::enableAttribute(GLuint index)
{
    glEnableVertexAttribArray(index);
}