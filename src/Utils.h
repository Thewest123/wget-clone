#pragma once

#include <string>
#include <algorithm>

namespace Utils
{
    /**
     * @brief Converts input 'str' to all lower case
     *
     * @param str Input string
     * @return std::string Output lower case string
     */
    std::string toLowerCase(const std::string &str);

    /**
     * @brief Returns true if input 'str' ends with a string 'ending'
     *
     * @param str Input string
     * @param ending Ending part string
     * @return true If 'input' ends with 'ending'
     * @return false In 'input' does NOT end with 'ending'
     */
    bool endsWith(const std::string &str, const std::string &ending);

    /**
     * @brief Returns true if input 'str' starts with a string 'beginning'
     *
     * @param str Input string
     * @param beginning Ending part string
     * @return true If 'input' starts with 'beginning'
     * @return false In 'input' does NOT start with 'beginning'
     */
    bool startsWith(const std::string &str, const std::string &beginning);

    /**
     * @brief Returns true if 'str' contains a 'part' string
     *
     * @param str Input string
     * @param part The part we're searching for
     * @return true If 'str' contains a 'part' string
     * @return false If 'str' does NOT contain a 'part' string
     */
    bool contains(const std::string &str, const std::string &part);
}