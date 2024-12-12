#include "MyuEngine/ScriptManager.h"
#include "MyuEngine/SpriteManager.h"

ScriptManager::ScriptManager() {
    Initialize();
}

void ScriptManager::Initialize() {
    lua.open_libraries(sol::lib::base, sol::lib::math);
    RegisterTypes();
}

void ScriptManager::RegisterTypes() {
    // Background 클래스 등록
    lua.new_usertype<Background>("Background",
        sol::constructors<Background(const std::string&)>(),
        sol::meta_function::construct, [](const std::string& textureName) {
            auto bg = new Background(textureName);
            auto sprite = bg->getSprite();
            sprite->setSortingOrder(0);  // 배경은 기본적으로 0
            SpriteManager::GetInstance().RegisterSprite(sprite);
            return bg;
        }
    );

    // Character 클래스 등록
    lua.new_usertype<Character>("Character",
        sol::constructors<Character(const std::string&, float, float)>(),
        "say", &Character::say,  // say 메서드 등록
        sol::meta_function::construct, [](const std::string& textureName, float x, float y) {
            auto chr = new Character(textureName, x, y);
            auto sprite = chr->getSprite();
            sprite->setSortingOrder(1);  // 캐릭터는 기본적으로 1
            SpriteManager::GetInstance().RegisterSprite(sprite);
            return chr;
        }
    );
}

bool ScriptManager::ExecuteFile(const std::string& filename) {
    try {
        lua.script_file(filename);
        return true;
    }
    catch (const sol::error& e) {
        std::cout << "Lua 오류: " << e.what() << std::endl;
        return false;
    }
}