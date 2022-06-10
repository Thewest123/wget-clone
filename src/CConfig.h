/**
 * @file CConfig.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Header file for CConfig
 *
 */

#pragma once

#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <iomanip>

using std::string, std::map;

/**
 * @brief Config singleton class to parse, store, and provide config values to other parts of the program
 *
 */
class CConfig
{
public:
    /**
     * @brief Struct holding the set values
     *
     */
    struct TSetting
    {
        TSetting();
        TSetting(const string &value);

        string m_Value;

        // Operators to convert to the righ type when getting values
        operator bool() const;
        operator int() const;
        operator string() const;

        // Operators to convert various inputs to uniform type (string) when setting the values
        TSetting &operator=(bool);
        TSetting &operator=(int);
        TSetting &operator=(const string &);
    };

    /**
     * @brief Construct a new CConfig object, setting the default values
     *
     */
    CConfig();

    /**
     * @brief Parse the CLI arguments and save to TSetting values
     *
     * @param argc Args Count
     * @param argv Args Values
     * @return true If parsed correctly
     * @return false If error occured, or to exit program
     */
    bool parseArgs(int argc, char const *argv[]);

    /**
     * @brief Operator [] to return the correct TSetting based on it's name
     *
     * @return CConfig::TSetting&
     */
    TSetting &operator[](const string &);

    // Singleton stuff:

    /**
     * @brief Get the singleton instance of CConfig
     *
     * @return CConfig&
     */
    static CConfig &getInstance();

    /**
     * @brief Disabled copy constructor because of CConfig being singleton
     *
     */
    CConfig(const CConfig &) = delete;

    /**
     * @brief Disabled operator= because of CConfig being singleton
     *
     */
    void operator=(const CConfig &) = delete;

private:
    map<string, TSetting> m_Settings;

    /**
     * @brief Prints basic usage and all available arguments
     *
     * @param programName
     */
    void
    printHelp(const string &programName) const;

    /**
     * @brief Formats the option to two columns for arguments and explanatory help
     *
     * @param paramSize First column size
     * @param args Arguments
     * @param helpText Explanatory help text
     * @return string Formatted string
     */
    string formatOption(size_t paramSize, const string &args, const string &helpText) const;

    /**
     * @brief Read next argument and set to configName TSetting
     *
     * @param configName Name of the TSetting config
     * @param currentArg Index of current argument
     * @param argc
     * @param argv
     * @return true If succesfully set
     * @return false If error - can't read next argument
     */
    bool setWithNext(const string &configName, int &currentArg, int argc, const char *argv[]);
};
