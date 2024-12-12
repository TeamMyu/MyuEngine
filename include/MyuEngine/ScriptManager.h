#pragma once
#include <sol/sol.hpp>
#include "Background.h"
#include "Character.h"

class ScriptManager {
public:
    ScriptManager();
    ~ScriptManager() = default;

    void Initialize();
    bool ExecuteFile(const std::string& filename);
    sol::state& GetLuaState() { return lua; }

private:
    sol::state lua;
    void RegisterTypes();
}; 