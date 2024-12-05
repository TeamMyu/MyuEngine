#pragma once

#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "VertexArrayObject.h"
#include "VertexBufferObject.h"
#include "Texture2D.h"

#include <memory>
#include <vector>

class UIElement : public std::enable_shared_from_this<UIElement>
{
public:
    UIElement(Shader& shader, const glm::vec2& position, const glm::vec2& size);
    virtual ~UIElement() = default;

    // 기본적인 변환 메서드들
    void setPosition(const glm::vec2& newPosition) { position = newPosition; }
    void setRotation(float angle) { rotation = angle; }
    void setScale(const glm::vec2& newScale) { scale = newScale; }
    void setColor(const glm::vec4& newColor) { color = newColor; }
    void setVisible(bool isVisible) { visible = isVisible; }

    // 상태 조회 메서드들
    glm::vec2 getPosition() const { return position; }
    float getRotation() const { return rotation; }
    glm::vec2 getScale() const { return scale; }
    glm::vec4 getColor() const { return color; }
    bool isVisible() const { return visible; }

    // 계층 구조 관련
    void addChild(std::shared_ptr<UIElement> child);
    void removeChild(std::shared_ptr<UIElement> child);
    void setParent(std::shared_ptr<UIElement> parent);

    // 렌더링
    virtual void draw();
    virtual void update(float dt);

protected:
    void initRenderData();
    glm::mat4 getTransformMatrix() const;

    Shader* shader;
    VertexArrayObject vao;
    VertexBufferObject vbo;

    glm::vec2 position;
    glm::vec2 scale;
    float rotation;
    glm::vec4 color;
    bool visible;

    std::weak_ptr<UIElement> parent;
    std::vector<std::shared_ptr<UIElement>> children;
}; 