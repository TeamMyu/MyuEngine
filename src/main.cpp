#include <iostream>
#include <filesystem>

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GL/Camera2D.h"
#include "GL/Shader.h"

#include "MyuEngine/CameraService.h"
#include "MyuEngine/ShaderManager.h"
#include "MyuEngine/ScriptManager.h"
#include "MyuEngine/SpriteManager.h"
#include "MyuEngine/ResourceManager.h"
#include "MyuEngine/DialogueManager.h"

namespace fs = std::filesystem;

int screenWidth = 800;
int screenHeight = 600;

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    screenWidth = width;
    screenHeight = height;
    glViewport(0, 0, width, height);
}

int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // 윈도우일 때, 콘솔에 출력할 글자 인코딩 설정
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
    #endif

    // OpenGL 버전 설정
#if defined(__APPLE__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#endif

    // 윈도우 성
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Preview", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // GLAD 초기화
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // GLAD 초기화 후에 추가
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto& shaderManager = ShaderManager::getInstance();

    // 스프라이트 기본 셰이더 로드
    if (!shaderManager.loadShader("sprite-default")) {
        return -1;
    }

    // UI 이미지 기본 셰이더 로드
    if (!shaderManager.loadShader("ui-image-default")) {
        return -1;
    }

    // 텍스 기본 셰이더 로드
    if (!shaderManager.loadShader("ui-text-default")) {
        return -1;
    }

    Camera2D camera(glm::vec2(0, 0), 2, 2);
    CameraService::registerMainCamera(&camera);

    // DialogueManager 초기화
    DialogueManager::GetInstance().initialize("dialogue_box", "NanumGothic");
    // DialogueManager::GetInstance().showDialogue("안녕하세요, World!");

    // 스프라이트 매니저 사용
    auto& spriteManager = SpriteManager::GetInstance();

    ScriptManager scriptManager;
    std::filesystem::path scriptPath = ResourceManager::getInstance()->getScriptsPath() / "main.lua";
    if (!scriptManager.ExecuteFile(scriptPath.generic_string())) {
        return -1;
    }

    // 메인 루프
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 정렬된 순서대로 모든 스프라이트 그리기
        for (const auto& sprite : spriteManager.GetSprites()) {
            sprite->draw();
        }

        DialogueManager::GetInstance().draw();

        glfwSwapBuffers(window);
    }

    // 정리
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}