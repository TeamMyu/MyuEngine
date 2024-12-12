#pragma once

#include <memory>
#include <string>

#include "GL/UIImage.h"
#include "GL/UIText.h"
#include "GL/Texture2D.h"

class DialogueManager {
public:
    static DialogueManager& GetInstance();

    void initialize(const std::string& dialogueBoxTexturePath, const std::string& fontPath);
    void showDialogue(const std::string& text);
    void hideDialogue();
    void draw();

private:
    DialogueManager() = default;
    ~DialogueManager() = default;

    // 싱글톤 패턴을 위한 복사/이동 방지
    DialogueManager(const DialogueManager&) = delete;
    DialogueManager& operator=(const DialogueManager&) = delete;
    DialogueManager(DialogueManager&&) = delete;
    DialogueManager& operator=(DialogueManager&&) = delete;

    std::shared_ptr<UIImage> dialogueBox;
    Texture2D dialogueBoxTexture;

    std::shared_ptr<UIText> dialogueText;

    bool isDialogueActive = false;
};
