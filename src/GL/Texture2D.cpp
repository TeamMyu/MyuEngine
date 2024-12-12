#include "stb_image.h"

#include "GL/Texture2D.h"

#include <iostream>

Texture2D::Texture2D() : id(0) {}

Texture2D::Texture2D(const std::string& path, GLint format, GLenum fileFormat)
{
    glGenTextures(1, &this->id);
    glBindTexture(GL_TEXTURE_2D, this->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);

    int width, height, nChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nChannels, 0);

    if (data)
    {
        this->width = width;
        this->height = height;
        glTexImage2D(GL_TEXTURE_2D, 0, format, this->width, this->height, 0, fileFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        std::cout << "Successfully loaded texture from: " << path << std::endl;
        std::cout << "Texture created at address: " << this << " with ID: " << this->id << std::endl;
    }
    else
    {
        this->width = 0;
        this->height = 0;
        std::cerr << "Failed to load texture: " << path << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
    }

    stbi_image_free(data);
}

Texture2D::~Texture2D()
{
    if (this->id != 0)
    {
        glDeleteTextures(1, &this->id);
        std::cout << "Texture destroyed at address: " << this << " with ID: " << this->id << std::endl;
    }
}

void Texture2D::bind(GLenum unit)
{
    glActiveTexture(unit);
    glBindTexture(GL_TEXTURE_2D, this->id);
}

Texture2D::Texture2D(Texture2D&& other) noexcept {
    id = other.id;                // 다른 객체의 리소스를 가져옴
    width = other.width;          // width 복사 추가
    height = other.height;        // height 복사 추가
    other.id = 0;                 // 다른 객체의 리소스를 무효화
    other.width = 0;              // width 초기화
    other.height = 0;             // height 초기화
}

Texture2D& Texture2D::operator=(Texture2D&& other) noexcept {
    if (this != &other) {
        if (this->id != 0) {      // 기존 리소스가 있다면
            glDeleteTextures(1, &this->id);  // 기존 리소스 해제
        }

        id = other.id;            // 새 리소스 가져옴
        width = other.width;      // width 복사 추가
        height = other.height;    // height 복사 추가
        other.id = 0;             // 원본 리소스 무효화
        other.width = 0;          // width 초기화
        other.height = 0;         // height 초기화
    }
    return *this;
}
