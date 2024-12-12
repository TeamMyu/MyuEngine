#include "MyuEngine/SpriteManager.h"
#include <algorithm>

SpriteManager& SpriteManager::GetInstance() {
    static SpriteManager instance;
    return instance;
}

void SpriteManager::RegisterSprite(std::shared_ptr<Sprite> sprite) {
    sprites.push_back(sprite);
    SortSprites();
}

const std::vector<std::shared_ptr<Sprite>>& SpriteManager::GetSprites() const {
    return sprites;
}

void SpriteManager::Clear() {
    sprites.clear();
}

void SpriteManager::SortSprites() {
    std::sort(sprites.begin(), sprites.end(),
        [](const std::shared_ptr<Sprite>& a, const std::shared_ptr<Sprite>& b) {
            return a->getSortingOrder() < b->getSortingOrder();
        });
}

std::vector<std::shared_ptr<Sprite>> SpriteManager::GetSpritesByOrder(int order) {
    std::vector<std::shared_ptr<Sprite>> result;
    for (const auto& sprite : sprites) {
        if (sprite->getSortingOrder() == order) {
            result.push_back(sprite);
        }
    }
    return result;
} 