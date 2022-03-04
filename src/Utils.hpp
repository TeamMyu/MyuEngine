#pragma once

#include <vector>
#include <fstream>
#include <stdexcept>

namespace Utils {
    std::vector<char> readFile(const std::string& filename);
}
