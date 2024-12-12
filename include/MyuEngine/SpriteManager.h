#pragma once
#include <vector>
#include <memory>
#include "GL/Sprite.h"

class SpriteManager {
public:
    static SpriteManager& GetInstance();

    // 스프라이트 등록 및 관리
    void RegisterSprite(std::shared_ptr<Sprite> sprite);
    const std::vector<std::shared_ptr<Sprite>>& GetSprites() const;
    void Clear();

    // 정렬 관련
    void SortSprites();
    std::vector<std::shared_ptr<Sprite>> GetSpritesByOrder(int order);

private:
    SpriteManager() = default;
    ~SpriteManager() = default;

    // 복사 및 이동 방지
    SpriteManager(const SpriteManager&) = delete;
    SpriteManager& operator=(const SpriteManager&) = delete;
    SpriteManager(SpriteManager&&) = delete;
    SpriteManager& operator=(SpriteManager&&) = delete;

    std::vector<std::shared_ptr<Sprite>> sprites;
}; 