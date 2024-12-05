#pragma once

#include "UIElement.h"
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

class UIText : public UIElement
{
public:
    UIText(Shader& shader, const std::string& text, const glm::vec2& position, float scale);
    ~UIText();

    void setText(const std::string& newText) { text = newText; }
    std::string getText() const { return text; }

    virtual void draw() override;
    void setFont(const std::string& fontPath);

private:
    struct Character {
        unsigned int textureID;
        glm::ivec2 size;
        glm::ivec2 bearing;
        unsigned int advance;
    };

    std::string text;
    FT_Library ft;
    FT_Face face;
    std::unordered_map<char, Character> characters;
};