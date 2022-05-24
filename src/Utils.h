#pragma once

#include <string>
#include <algorithm>

namespace Utils
{
    std::string toLowerCase(const std::string &str);
    bool endsWith(const std::string &str, const std::string &ending);
}