#include "GL/Sprite.h"

Sprite::Sprite(Texture2D& texture, Shader& shader, glm::vec3 position, glm::vec3 size)
    : texture(&texture)
    , shader(&shader)
    , position(position)
    , scale(size)
    , rotation(0.0f)
    , color(1.0f)
{
    // 중심이 (0,0,0)이 되도록 버텍스 데이터 수정
    std::vector<float> vertices = {
        // 위치              // 텍스처 좌표
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,  // 좌상단
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f,  // 우하단
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,  // 좌하단

        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,  // 좌상단
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f,  // 우상단
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f   // 우하단
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

void Sprite::draw() {
    shader->use();

    // 단순화된 모델 행렬 계산
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);    // 위치 변환
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));    // 회전
    model = glm::scale(model, scale);    // 크기 변환

    // 셰이더 유니폼 설정
    shader->setMatrix4f("model", model);
    shader->setVector4f("spriteColor", color);

    // 텍스처 바인딩
    texture->bind(GL_TEXTURE0);

    // 렌더링
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao.unbind();
}
