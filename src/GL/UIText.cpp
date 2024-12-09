#include "GL/UIText.h"
#include <iostream>

UIText::UIText(Shader& shader, const std::wstring& text, const glm::vec2& position, float scale)
    : UIElement(shader, position, glm::vec2(scale))
    , text(text)
    , ft(nullptr)
    , face(nullptr)
{
    // FreeType 초기화
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    // 문자 맵 초기화 추가
    characters.clear();

    // VAO, VBO 초기화
    vao.bind();
    vbo.bind();

    // 동적 텍스트를 위한 VBO 설정
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    // 위치 속성
    vao.setAttributePointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    vao.enableAttribute(0);

    vao.unbind();
}

UIText::~UIText()
{
    // 문자 텍스처 정리
    for (auto& pair : characters) {
        glDeleteTextures(1, &pair.second.textureID);
    }

    // FreeType 정리
    if (face)
        FT_Done_Face(face);
    if (ft)
        FT_Done_FreeType(ft);
}

void UIText::loadCharacter(FT_Face &face, wchar_t c)
{
    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
        std::cout << " failed load font glyph: " << c << std::endl;
        return;
    }

    // 텍스처 생성
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    if (face->glyph->bitmap.buffer)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
    }
    else // 버퍼가 비어있을 경우, 1x1 픽셀 크기의 텍스처를 생성
    {
        unsigned char emptyPixel = 0;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE, &emptyPixel);
    }

    // 텍스처 설정
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 문자 정보 저장
    Character character = {
        texture,
        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
        static_cast<unsigned int>(face->glyph->advance.x)
    };
    characters.insert(std::pair<wchar_t, Character>(c, character));
}

void UIText::setFont(const std::string& fontPath)
{
    // 이전 face가 있다면 정리
    if (face) {
        FT_Done_Face(face);
    }

    // 폰트 로드
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }

    // 폰트 크기 설정
    FT_Set_Pixel_Sizes(face, 0, 48);

    // 텍스처 설정
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 모든 ASCII 문자에 대해 글리프 로드
    for (wchar_t c = 0; c < 128; c++)
        loadCharacter(face, c);

    // 한글 문자에 대해 글리프 로드
    for (wchar_t c = 0xAC00; c < 0xD7A3; c++)
        loadCharacter(face, c);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // 원복시키기
}

void UIText::draw()
{
    if (!isVisible()) return;
    if (characters.empty()) return;  // 문자 맵이 비어있으면 그리지 않음

    shader->use();
    shader->setMatrix4f("model", getTransformMatrix());
    shader->setVector4f("textColor", getColor());

    glActiveTexture(GL_TEXTURE0);
    vao.bind();

    float x = 0.0f;
    float y = 0.0f;
    float scale = 1.0f;

    // 각 문자 렌더링
    for (wchar_t c : text) {
        Character ch = characters[c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        // 각 문자에 대한 VBO 업데이트
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        // 문자 텍스처 렌더링
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        vbo.bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 다음 글리프를 위해 위치 업데이트
        x += (ch.advance >> 6) * scale;
    }

    vao.unbind();
    glBindTexture(GL_TEXTURE_2D, 0);
}