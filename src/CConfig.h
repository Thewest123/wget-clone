/**
 * @file CConfig.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Config singleton class to parse, store, and provide config values to other parts of the program
 *
 */

#pragma once

#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "CLogger.h"

using namespace std;

class CConfig
{
public:
    /**
     * @brief Struct holding the set values
     *
     */
    struct Setting
    {
        Setting();
        Setting(const string &value);

        string m_Value;

        // Operators to convert to the righ type when getting values
        operator bool() const;
        operator int() const;
        operator string() const;

        // Operators to convert various inputs to uniform type (string) when setting the values
        Setting &operator=(bool);
        Setting &operator=(int);
        Setting &operator=(const string &);
    };

    /**
     * @brief Construct a new CConfig object, setting the default values
     *
     */
    CConfig();

    /**
     * @brief Parse the CLI arguments and save to Setting values
     *
     * @param argc Args Count
     * @param argv Args Values
     * @return true If parsed correctly
     * @return false If error occured, or to exit program
     */
    bool parseArgs(int argc, char const *argv[]);

    /**
     * @brief Operator [] to return the correct Setting based on it's name
     *
     * @return CConfig::Setting&
     */
    Setting &
    operator[](const string &);

    // Singleton stuff

    /**
     * @brief Get the singleton instance of CConfig
     *
     * @return CConfig&
     */
    static CConfig &getInstance();

    // Disable copy constructor and operator= because of CConfig being singleton

    CConfig(const CConfig &) = delete;
    void operator=(const CConfig &) = delete;

private:
    map<string, Setting> m_Settings;

    /**
     * @brief Prints basic usage and all available arguments
     *
     * @param programName
     */
    void printHelp(const string &programName);
};