#include "GL/UIImage.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "MyuEngine/WindowManager.h"

UIImage::UIImage(Shader& shader, Texture2D& texture, const glm::vec2& position, const glm::vec2& size)
    : UIElement(shader, position, size)
    , texture(&texture)
{
}

void UIImage::setTexture(Texture2D& newTexture)
{
    texture = &newTexture;
}

void UIImage::draw()
{
    if (!isVisible()) return;

    shader->use();
    shader->setMatrix4f("model", getTransformMatrix());
    shader->setVector4f("color", getColor());

    auto& wm = WindowManager::getInstance();
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(wm.getWidth()), static_cast<float>(wm.getHeight()), 0.0f, -1.0f, 1.0f);
    shader->setMatrix4f("projection", projection);

    // 텍스처 바인딩
    texture->bind(GL_TEXTURE0);

    // 렌더링
    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao.unbind();

    // 자식 요소들 렌더링
    for (const auto& child : children) {
        child->draw();
    }
}