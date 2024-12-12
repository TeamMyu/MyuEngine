#pragma once

#include <filesystem>

class ResourceManager {
private:
    static ResourceManager* instance;
    std::filesystem::path resourcePath;

    ResourceManager();  // private 생성자

public:
    // 복사 생성자와 대입 연산자를 삭제
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    static ResourceManager* getInstance();
    std::filesystem::path getTexturesPath();
    std::filesystem::path getScriptsPath();
    std::filesystem::path getFontsPath();

    // 소멸자
    ~ResourceManager();
};