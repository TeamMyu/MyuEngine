#include "MyuEngine/ResourceManager.h"

ResourceManager* ResourceManager::instance = nullptr;

ResourceManager::ResourceManager() {
    resourcePath = std::filesystem::path(__FILE__).parent_path() / ".." / ".." / "resources";
}

ResourceManager* ResourceManager::getInstance() {
    if (instance == nullptr) {
        instance = new ResourceManager();
    }
    return instance;
}

std::filesystem::path ResourceManager::getTexturesPath() {
    return resourcePath / "textures";
}

std::filesystem::path ResourceManager::getScriptsPath() {
    return resourcePath / "scripts";
}

std::filesystem::path ResourceManager::getFontsPath() {
    return resourcePath / "fonts";
}

ResourceManager::~ResourceManager() {
    // 필요한 정리 작업 수행
}