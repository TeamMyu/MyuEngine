#pragma once

#include <string>
#include <iostream>

namespace Myu
{
    class Debug
    {
    public:
        static void Log(std::string msg)
        {
            std::cout << msg << std::endl;
        }
    };
}
