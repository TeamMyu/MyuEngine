#pragma once

#include "UIElement.h"
#include "Texture2D.h"

class UIImage : public UIElement
{
public:
    UIImage(Shader& shader, Texture2D& texture, const glm::vec2& position, const glm::vec2& size);
    ~UIImage() = default;

    void setTexture(Texture2D& newTexture);
    Texture2D* getTexture() const { return texture; }

    virtual void draw() override;

private:
    Texture2D* texture;
}; 