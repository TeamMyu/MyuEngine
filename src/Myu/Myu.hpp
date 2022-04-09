#pragma once

#include "Application.hpp"

#define MYUAPI __declspec(dllexport)


MYUAPI Application* CreateApplication()
{
    return new Application();
};

