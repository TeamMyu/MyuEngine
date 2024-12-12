#include "MyuEngine/Background.h"
#include "MyuEngine/ShaderManager.h"
#include "MyuEngine/WindowManager.h"
#include "MyuEngine/ResourceManager.h"
#include <GLFW/glfw3.h>

extern int screenWidth;
extern int screenHeight;

Background::Background(const std::string& textureName)
{
    std::filesystem::path texturesPath = ResourceManager::getInstance()->getTexturesPath();
    texture = Texture2D((texturesPath / (textureName + ".jpg")).generic_string(), GL_RGB, GL_RGB);

    auto& shaderManager = ShaderManager::getInstance();
    auto shader = shaderManager.getShader("sprite-default");

    // Sprite 생성 (위치는 중앙(0,0,0), 크기는 updateSpriteSizeToScreen에서 설정)
    sprite = std::make_shared<Sprite>(
        texture,
        shader,
        glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3(1.0f)
    );

    // 배경이므로 가장 뒤에 그려지도록 설정
    sprite->setSortingOrder(-100);

    // 초기 크기 설정
    updateSpriteSizeToScreen();
}

void Background::draw() {
    updateSpriteSizeToScreen();
    sprite->draw();
}

void Background::setTexture(const std::string& texturePath) {
    texture = Texture2D(texturePath, GL_RGB, GL_RGB);
}

void Background::updateSpriteSizeToScreen() {
    // 화면 비율에 맞게 스프라이트 크기 조정
    auto& wm = WindowManager::getInstance();
    float spriteHeight = 2.0f; // OpenGL 좌표계에서 화면 높이
    float spriteWidth = spriteHeight * wm.getAspectRatio();

    sprite->setScale(glm::vec3(spriteWidth, spriteHeight, 1.0f));
}