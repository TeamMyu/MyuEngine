#pragma once
#include "GL/Shader.h"

#include <unordered_map>
#include <memory>
#include <string>
#include <filesystem>

class ShaderManager {
public:
    static ShaderManager& getInstance();

    // 새로운 셰이더 로드
    bool loadShader(const std::string& name);

    // 셰이더 가져오기
    std::shared_ptr<Shader> getShader(const std::string& name);

    // 모든 셰이더 제거
    void clearAllShaders();

    // 특정 셰이더 제거
    void removeShader(const std::string& name);

private:
    ShaderManager();

    ~ShaderManager() = default;

    // 복사 및 할당 방지
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
    std::filesystem::path shadersPath;
};