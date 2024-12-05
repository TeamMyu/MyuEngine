#pragma once

#include <vector>
#include <glad/glad.h>

class IndexBufferObject
{
public:
    IndexBufferObject();
    ~IndexBufferObject();

    IndexBufferObject(const IndexBufferObject&) = delete;
    IndexBufferObject& operator=(const IndexBufferObject&) = delete;

    void bind() const;

    void unbind() const;

    void write(const std::vector<float>& indices);

private:
    GLuint id;
};
