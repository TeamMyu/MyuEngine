#include "MyuEngine/ShaderManager.h"
#include <iostream>
#include <filesystem>

ShaderManager& ShaderManager::getInstance() {
    static ShaderManager instance;
    return instance;
}

ShaderManager::ShaderManager() {
    shadersPath = std::filesystem::path(__FILE__).parent_path() / ".." / ".." / "resources" / "shaders";
}

bool ShaderManager::loadShader(const std::string& name) {
    // 파일 이름 구성
    auto vertexPath = shadersPath / (name + ".vert");
    auto fragmentPath = shadersPath / (name + ".frag");

    // 파일 존재 여부 확인
    if (!std::filesystem::exists(vertexPath) || !std::filesystem::exists(fragmentPath)) {
        std::cerr << "Shader files for '" << name << "' not found!" << std::endl;
        return false;
    }

    try {
        shaders[name] = std::make_shared<Shader>(
            vertexPath.generic_string(),
            fragmentPath.generic_string()
        );
        std::cout << "Shader '" << name << "' loaded successfully" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load shader '" << name << "': " << e.what() << std::endl;
        return false;
    }
}

std::shared_ptr<Shader> ShaderManager::getShader(const std::string& name) {
    auto it = shaders.find(name);
    if (it != shaders.end()) {
        return it->second;
    }
    std::cerr << "Shader '" << name << "' not found!" << std::endl;
    return nullptr;
}

void ShaderManager::clearAllShaders() {
    shaders.clear();
    std::cout << "All shaders have been cleared" << std::endl;
}

void ShaderManager::removeShader(const std::string& name) {
    auto it = shaders.find(name);
    if (it != shaders.end()) {
        shaders.erase(it);
        std::cout << "Shader '" << name << "' removed" << std::endl;
    }
}