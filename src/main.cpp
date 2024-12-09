#include "GL/Texture2D.h"
#include "GL/Shader.h"
#include "GL/Sprite.h"
#include "GL/Camera2D.h"
#include "GL/UIImage.h"
#include "GL/UIText.h"
#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <memory>

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

    // 자주 사용 할 경로 미리 세팅
    fs::path texturesPath = fs::path(__FILE__).parent_path().parent_path() / "resources" / "textures";
    fs::path shadersPath = fs::path(__FILE__).parent_path().parent_path() / "resources" / "shaders";
    fs::path fontsPath = fs::path(__FILE__).parent_path().parent_path() / "resources" / "fonts";

    // 셰이더와 텍스처 로드
    Shader uiShader((shadersPath / "ui.vert").string(), (shadersPath / "ui.frag").string());
    Texture2D imageTexture((texturesPath / "ui_test.png").string(), GL_RGBA, GL_RGBA);

    // UI 이미지 생성
    auto image = std::make_shared<UIImage>(
        uiShader,
        imageTexture,
        glm::vec2(0.0f, 400.0f),  // 위치
        glm::vec2(800.0f, 150.0f)   // 크기
    );
    image->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 0.8f));  // 약간 투명하게

    Texture2D anyaTexture((texturesPath / "anya.jpg").string(), GL_RGB, GL_RGB);
    Texture2D tomoriTexture((texturesPath / "tomori.jpg").string(), GL_RGB, GL_RGB);

    Shader spriteShader((shadersPath / "sprite.vert").string(), (shadersPath / "sprite.frag").string());
    spriteShader.use();
    spriteShader.setInt("mainTexture", 0);

    Camera2D camera(glm::vec2(0, 0), 2, 2);
    spriteShader.setMatrix4f("projection", camera.getProjectionMatrix());

    // 일반 벡터 사용
    std::vector<std::unique_ptr<Sprite>> sprites;

    // 스프라이트 추가
    sprites.push_back(std::make_unique<Sprite>(anyaTexture, spriteShader,
        glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
    sprites.back()->setSortingOrder(0);

    sprites.push_back(std::make_unique<Sprite>(tomoriTexture, spriteShader,
        glm::vec3(0.25f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)));
    sprites.back()->setSortingOrder(1);

    // 정렬
    std::sort(sprites.begin(), sprites.end(),
        [](const auto& a, const auto& b) {
            return a->getSortingOrder() < b->getSortingOrder();
    });

    // 텍스트 셰이더 로드
    Shader textShader((shadersPath / "text.vert").string(), (shadersPath / "text.frag").string());
    textShader.use();
    textShader.setInt("text", 0);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(screenWidth),
                                     0.0f, static_cast<float>(screenHeight),
                                     -1.0f, 1.0f);
    textShader.setMatrix4f("projection", projection);

    // UI 텍스트 생성
    auto text = std::make_shared<UIText>(
        textShader,
        L"안녕하세요, World!",
        glm::vec2(0.0f, 300.0f),
        1.0f
    );
    text->setFont((fontsPath / "NanumGothic.ttf").string());  // setFont가 성공적으로 호출되었는지 확인
    text->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

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

        text->draw();

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