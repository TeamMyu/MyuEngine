#include "GL/UIImage.h"

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