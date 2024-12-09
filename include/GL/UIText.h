#pragma once

#include "UIElement.h"
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

class UIText : public UIElement
{
public:
    UIText(Shader& shader, const std::wstring& text, const glm::vec2& position, float scale);
    ~UIText();

    void setText(const std::wstring& newText) { text = newText; }
    std::wstring getText() const { return text; }

    virtual void draw() override;
    void setFont(const std::string& fontPath);

private:
    struct Character {
        unsigned int textureID;
        glm::ivec2 size;
        glm::ivec2 bearing;
        unsigned int advance;
    };

    void loadCharacter(FT_Face &face, wchar_t c);

    std::wstring text;
    FT_Library ft;
    FT_Face face;
    std::unordered_map<wchar_t, Character> characters;
};
