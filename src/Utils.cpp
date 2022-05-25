#include "Utils.h"

#include <string>
#include <algorithm>

#include <iostream>

std::string Utils::toLowerCase(const std::string &str)
{
    std::string strLower = str;

    // Transform to lowercase
    transform(strLower.begin(), strLower.end(), strLower.begin(),
              [](unsigned char c)
              {
                  return std::tolower(c);
              });

    return strLower;
}

bool Utils::endsWith(const std::string &str, const std::string &ending)
{
    if (ending.size() > str.size())
        return false;

    return std::equal(ending.rbegin(), ending.rend(), str.rbegin());
}

bool Utils::startsWith(const std::string &str, const std::string &beginning)
{
    return str.rfind(beginning, 0) == 0;
}