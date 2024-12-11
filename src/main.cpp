#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <memory>

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "GL/Camera2D.h"
#include "GL/Shader.h"
#include "GL/Sprite.h"
#include "GL/Texture2D.h"
#include "GL/UIImage.h"
#include "GL/UIText.h"

#include "MyuEngine/CameraService.h"
#include "MyuEngine/ShaderManager.h"
#include "MyuEngine/Character.h"
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

    // 윈도우 생성
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "OpenGL Window", nullptr, nullptr);
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

    Camera2D camera(glm::vec2(0, 0), 2, 2);
    CameraService::registerMainCamera(&camera);

    // Texture2D imageTexture((texturesPath / "ui_test.png").string(), GL_RGBA, GL_RGBA);

    // // UI 이미지 생성
    // auto image = std::make_shared<UIImage>(
    //     uiShader,
    //     imageTexture,
    //     glm::vec2(0.0f, 400.0f),  // 위치
    //     glm::vec2(800.0f, 150.0f)   // 크기
    // );
    // image->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.8f));  // 약간 투명하게

    // Texture2D tomoriTexture((texturesPath / "tomori.jpg").string(), GL_RGB, GL_RGB);

    Character taigaCharacter("taiga", glm::vec2(0.0f, 0.0f));

    std::vector<std::shared_ptr<Sprite>> sprites;
    sprites.push_back(taigaCharacter.getSprite());

    // 정렬
    std::sort(sprites.begin(), sprites.end(),
        [](const auto& a, const auto& b) {
            return a->getSortingOrder() < b->getSortingOrder();
    });

    // // 텍스트 셰이더 로드
    // Shader textShader((shadersPath / "text.vert").string(), (shadersPath / "text.frag").string());
    // textShader.use();
    // textShader.setInt("text", 0);
    // glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(screenWidth),
    //                                  0.0f, static_cast<float>(screenHeight),
    //                                  -1.0f, 1.0f);
    // textShader.setMatrix4f("projection", projection);

    // // UI 텍스트 생성
    // auto text = std::make_shared<UIText>(
    //     textShader,
    //     L"안녕하세요, World!",
    //     glm::vec2(0.0f, 300.0f),
    //     1.0f
    // );
    // text->setFont((fontsPath / "NanumGothic.ttf").string());  // setFont가 성공적으로 호출되었는지 확인
    // text->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    // 메인 루프
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 그리기
        for (auto& sprite : sprites) {
            sprite->draw();
        }

        // text->draw();

        // glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(screenWidth),
        //                                   static_cast<float>(screenHeight), 0.0f, -1.0f, 1.0f);
        // uiShader.use();
        // uiShader.setMatrix4f("projection", projection);
        // image->draw();

        glfwSwapBuffers(window);
    }

    // 정리
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}