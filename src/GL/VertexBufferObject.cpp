#include "GL/VertexBufferObject.h"

VertexBufferObject::VertexBufferObject()
{
    glGenBuffers(1, &id);
}

VertexBufferObject::~VertexBufferObject()
{
    glDeleteBuffers(1, &id);
}

void VertexBufferObject::bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, id);
}

void VertexBufferObject::unbind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBufferObject::write(const std::vector<float>& vertices)
{
    bind();
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices.front(), GL_STATIC_DRAW);
}

