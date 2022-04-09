#pragma once

#include "Core/Application.hpp"

#define MYUAPI __declspec(dllexport)


MYUAPI Application* CreateApplication()
{
    return new Application();
};

