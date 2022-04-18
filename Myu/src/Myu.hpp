#pragma once

#include "Core/Application.hpp"

#ifdef MYU_BUILD_DLL
    #define MYU_API __declspec(dllexport)
#else
    #define MYU_API __declspec(dllimport)
#endif

MYU_API Myu::Application* CreateApplication()
{
    return new Myu::Application();
};
