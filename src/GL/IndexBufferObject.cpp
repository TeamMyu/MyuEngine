#include "GL/IndexBufferObject.h"

IndexBufferObject::IndexBufferObject()
{
    glGenBuffers(1, &this->id);
}

IndexBufferObject::~IndexBufferObject()
{
    glDeleteBuffers(1, &this->id);
}

void IndexBufferObject::bind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->id);
}

void IndexBufferObject::unbind() const
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBufferObject::write(const std::vector<float>& indices)
{
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(float), indices.data(), GL_STATIC_DRAW);
}