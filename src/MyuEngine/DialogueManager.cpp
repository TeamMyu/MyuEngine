#include <codecvt>
#include <locale>


#include "MyuEngine/DialogueManager.h"
#include "MyuEngine/ShaderManager.h"
#include "MyuEngine/WindowManager.h"
#include "MyuEngine/ResourceManager.h"

DialogueManager& DialogueManager::GetInstance() {
    static DialogueManager instance;
    return instance;
}

void DialogueManager::initialize(const std::string& textureName, const std::string& fontName) {
    std::shared_ptr<Shader> shader = ShaderManager::getInstance().getShader("ui-image-default");
    if (!shader) {
        throw std::runtime_error("Failed to get shader 'ui-image-default'");
    }

    std::filesystem::path texturesPath = ResourceManager::getInstance()->getTexturesPath();
    dialogueBoxTexture = Texture2D((texturesPath / (textureName + ".png")).generic_string(), GL_RGBA, GL_RGBA);

    auto& wm = WindowManager::getInstance();
    float screenWidth = static_cast<float>(wm.getWidth());
    float screenHeight = static_cast<float>(wm.getHeight());

    dialogueBox = std::make_shared<UIImage>(
        *shader,
        dialogueBoxTexture,
        glm::vec3(screenWidth * 0.125f, screenHeight * 0.75f, 0.0f),
        // glm::vec2(screenWidth * 0.5f, screenHeight * 0.15f),
        glm::vec2(screenWidth * 0.8f, screenHeight * 0.2f)
    );

    auto textShader = ShaderManager::getInstance().getShader("ui-text-default");
    dialogueText = std::make_shared<UIText>(
        *textShader,
        L"",
        glm::vec2(100.0f, 200.0f),
        1.0f
    );
    dialogueText->setFont(fontName);
    dialogueText->setColor(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));  // 흰색으로 설정
}

void DialogueManager::showDialogue(const std::string& text) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wtext = converter.from_bytes(text);

    dialogueText->setText(wtext);
    isDialogueActive = true;
}

void DialogueManager::hideDialogue() {
    isDialogueActive = false;
}

void DialogueManager::draw() {
    if (!isDialogueActive) return;

    dialogueBox->draw();
    dialogueText->draw();
}