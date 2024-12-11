#pragma once

#include "glad/glad.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Texture2D.h"
#include "VertexArrayObject.h"
#include "VertexBufferObject.h"
#include "Shader.h"

class Sprite
{
public:
    Sprite(Texture2D& texture, std::shared_ptr<Shader> shader, glm::vec3 position, glm::vec3 size);
    Sprite(Texture2D& texture, glm::vec3 position, glm::vec3 size, glm::vec3 rotation);

    ~Sprite() = default;

    // 기본적인 변환 메서드들
    inline void setPosition(const glm::vec3& newPosition) { position = newPosition; }
    inline void setRotation(float angle) { rotation = angle; }
    inline void setScale(const glm::vec3& newScale) { scale = newScale; }

    // 상태 조회 메서드들
    inline glm::vec3 getPosition() const { return position; }
    inline float getRotation() const { return rotation; }
    inline glm::vec3 getScale() const { return scale; }

    // 렌더링 관련
    void draw();
    inline void setColor(const glm::vec4& newColor) { color = newColor; }

    // 셰이더 관련 메서드 추가
    inline void setShader(std::shared_ptr<Shader> newShader) { shader = newShader; }
    inline std::shared_ptr<Shader> getShader() const { return shader; }

    // Sorting order 관련 메서드 추가
    inline void setSortingOrder(int order) { sortingOrder = order; }
    inline int getSortingOrder() const { return sortingOrder; }

private:
    Texture2D* texture;
    std::shared_ptr<Shader> shader;
    glm::vec3 position;
    glm::vec3 scale;
    float rotation;
    glm::vec4 color;

    VertexArrayObject vao;
    VertexBufferObject vbo;

    int sortingOrder{0};
};