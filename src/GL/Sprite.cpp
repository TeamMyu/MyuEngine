#include "GL/Sprite.h"
#include "MyuEngine/ShaderManager.h"
#include "MyuEngine/CameraService.h"

Sprite::Sprite(Texture2D& texture, std::shared_ptr<Shader> shader, glm::vec3 position, glm::vec3 size)
    : texture(&texture)
    , shader(shader)
    , position(position)
    , scale(size)
    , rotation(0.0f)
    , color(1.0f)
{
    // 텍스처의 가로세로 비율 계산
    float aspectRatio = static_cast<float>(texture.getWidth()) / static_cast<float>(texture.getHeight());

    // 가로세로 비율을 고려한 버텍스 데이터 생성
    std::vector<float> vertices = {
        // 위치                           // 텍스처 좌표
        -0.5f * aspectRatio,  0.5f, 0.0f,   0.0f, 1.0f,  // 좌상단
         0.5f * aspectRatio, -0.5f, 0.0f,   1.0f, 0.0f,  // 우하단
        -0.5f * aspectRatio, -0.5f, 0.0f,   0.0f, 0.0f,  // 좌하단

        -0.5f * aspectRatio,  0.5f, 0.0f,   0.0f, 1.0f,  // 좌상단
         0.5f * aspectRatio,  0.5f, 0.0f,   1.0f, 1.0f,  // 우상단
         0.5f * aspectRatio, -0.5f, 0.0f,   1.0f, 0.0f   // 우하단
    };

    // VBO 설정
    vbo.write(vertices);

    // VAO 설정
    vao.bind();

    // 위치 속성
    vao.setAttributePointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    vao.enableAttribute(0);

    // 텍스처 좌표 속성
    vao.setAttributePointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    vao.enableAttribute(1);

    vao.unbind();
}

Sprite::Sprite(Texture2D& texture, glm::vec3 position, glm::vec3 size, glm::vec3 rotation)
    : Sprite(texture, ShaderManager::getInstance().getShader("sprite-default"), position, size)
{
}

void Sprite::draw() {
    // 1. 셰이더 활성화
    shader->use();

    // 2. 텍스처 바인딩 (셰이더 유니폼 설정 전에)
    texture->bind(GL_TEXTURE0);

    // 3. 셰이더 유니폼 설정
    shader->setInt("mainTex", 0);

    // 모델 행렬 계산 및 설정
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, scale);

    shader->setMatrix4f("model", model);
    shader->setVector4f("spriteColor", color);

    if (auto* camera = CameraService::getMainCamera())
        shader->setMatrix4f("projection", camera->getProjectionMatrix());
    else
        std::cerr << "메인 카메라가 등록되지 않았습니다!" << std::endl;

    // 4. 렌더링
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao.unbind();
}
