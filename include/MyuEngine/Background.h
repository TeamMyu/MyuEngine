#pragma once

#include "GL/Sprite.h"
#include "GL/Texture2D.h"
#include <memory>

class Background {
public:
    Background(const std::string& texturePath);
    ~Background() = default;

    // 복사 생성자와 대입 연산자 삭제
    Background(const Background&) = delete;
    Background& operator=(const Background&) = delete;

    // 이동 생성자와 대입 연산자는 기본값 사용
    Background(Background&&) = default;
    Background& operator=(Background&&) = default;

    void draw();
    void setTexture(const std::string& texturePath);
    std::shared_ptr<Sprite> getSprite() const { return sprite; }

private:
    Texture2D texture;
    std::shared_ptr<Sprite> sprite;
    void updateSpriteSizeToScreen();
};