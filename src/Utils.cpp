/**
 * @file Utils.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Implementation of Utils namespace functions
 *
 */

#include "Utils.h"

#include <string>
#include <algorithm>
#include <iostream>

std::string Utils::toLowerCase(const std::string &str)
{
    std::string strLower = str;

    // Transform to lowercase using lambda with unsigned char
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

bool Utils::contains(const std::string &str, const std::string &part)
{
    return str.find(part) != std::string::npos;
}

size_t Utils::replaceAll(std::string &str, const std::string &what, const std::string &to)
{
    size_t index = 0;
    size_t count = 0;

    if (what.empty())
        throw std::invalid_argument("'what' argument cannot be empty!");

    if (what == str)
        throw std::invalid_argument("'what' argument cannot be equal to 'str'!");

    // @inspiredBy https://stackoverflow.com/a/4643526
    while (true)
    {
        // Locate the substring to replace
        index = str.find(what, index);
        if (index == std::string::npos)
            break;

        // Make the replacement
        size_t whatLength = what.length();
        str.replace(index, whatLength, to);

        // Advance index forward so the next iteration doesn't pick it up as well
        index += whatLength;
        count++;
    }

    return count;
}

std::vector<std::string> Utils::splitString(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> lines;

    auto start = 0U;
    auto end = str.find(delimiter);

    while (end != std::string::npos)
    {
        lines.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }

    lines.push_back(str.substr(start, end));

    return lines;
}
