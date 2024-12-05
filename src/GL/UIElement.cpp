#include "GL/UIElement.h"

UIElement::UIElement(Shader& shader, const glm::vec2& position, const glm::vec2& size)
    : shader(&shader)
    , position(position)
    , scale(size)
    , rotation(0.0f)
    , color(1.0f)
    , visible(true)
{
    initRenderData();
}

void UIElement::initRenderData()
{
    float vertices[] = {
        // pos      // tex
        0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f,

        0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f
    };

    vao.bind();
    vbo.bind();
    vbo.write(std::vector<float>(vertices, vertices + sizeof(vertices) / sizeof(float)));

    vao.setAttributePointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    vao.enableAttribute(0);

    vao.unbind();
}

glm::mat4 UIElement::getTransformMatrix() const
{
    glm::mat4 transform = glm::mat4(1.0f);

    if (auto parentPtr = parent.lock()) {
        transform = parentPtr->getTransformMatrix();
    }

    transform = glm::translate(transform, glm::vec3(position, 0.0f));

    transform = glm::translate(transform, glm::vec3(0.5f * scale.x, 0.5f * scale.y, 0.0f));
    transform = glm::rotate(transform, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    transform = glm::translate(transform, glm::vec3(-0.5f * scale.x, -0.5f * scale.y, 0.0f));

    transform = glm::scale(transform, glm::vec3(scale, 1.0f));

    return transform;
}

void UIElement::draw()
{
    if (!visible) return;

    shader->use();
    shader->setMatrix4f("model", getTransformMatrix());
    shader->setVector4f("color", color);

    vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    vao.unbind();

    for (const auto& child : children) {
        child->draw();
    }
}

void UIElement::update(float dt)
{
    for (const auto& child : children) {
        child->update(dt);
    }
}

void UIElement::addChild(std::shared_ptr<UIElement> child)
{
    if (child) {
        child->setParent(shared_from_this());
        children.push_back(child);
    }
}

void UIElement::removeChild(std::shared_ptr<UIElement> child)
{
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        (*it)->setParent(nullptr);
        children.erase(it);
    }
}

void UIElement::setParent(std::shared_ptr<UIElement> newParent)
{
    parent = newParent;
}