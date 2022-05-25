/**
 * @file CConfig.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include "CConfig.h"

using namespace std;

CConfig::CConfig()
{
    (*this)["url"] = "";
    (*this)["depth"] = 1;
    (*this)["remote_images"] = false;
    (*this)["error_page"] = false;
    (*this)["output"] = string("./output");
    (*this)["log_level"] = 1;
    (*this)["cookies"] = string("");
    (*this)["user_agent"] = string("WGET-Project/0.1 (FIT CVUT, Jan Cerny <cernyj87@fit.cvut.cz>)");
};

bool CConfig::parseArgs(int argc, char const *argv[])
{
    auto &logger = CLogger::getInstance();

    // Check args count
    if (argc < 2)
    {
        logger.log(CLogger::LogLevel::Error, "Too few arguments!");
        return false;
    }

    // Loop through all remaining arguments
    for (int i = 1; i < argc; i++)
    {
        string value = argv[i];

        if (value == "-h" || value == "--help")
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: Read Help argument");
            return false;
        }

        else if (value == "-d" || value == "--depth")
        {
            value = argv[++i];

            if (value.find_first_not_of("0123456789") != string::npos)
            {
                logger.log(CLogger::LogLevel::Error, "Depth value is not a valid number!");
                return false;
            }

            logger.log(CLogger::LogLevel::Verbose, "Config: depth = " + value);
            (*this)["depth"] = value;
        }

        else if (value == "-r" || value == "--remote-images")
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: remote_images = true");
            (*this)["remote_images"] = true;
        }

        else if (value == "-e" || value == "--error-page")
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: error_page = true");
            (*this)["error_page"] = true;
        }

        else if (value == "-o" || value == "--output")
        {
            value = argv[++i];

            logger.log(CLogger::LogLevel::Verbose, "Config: output = " + value);
            (*this)["output"] = value;
        }

        else if (value == "-v" || value == "--verbose")
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: log_level = verbose");
            (*this)["log_level"] = 0;
            logger.setLevel(CLogger::LogLevel::Verbose);
        }

        else if (value == "-q" || value == "--quiet")
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: log_level = error");
            (*this)["log_level"] = 2;
            logger.setLevel(CLogger::LogLevel::Error);
        }

        else if (value == "-c" || value == "--cookies")
        {
            value = argv[++i];

            logger.log(CLogger::LogLevel::Verbose, "Config: cookies = " + value);
            (*this)["cookies"] = value;
        }

        else if (value == "-u" || value == "--user-agent")
        {
            value = argv[++i];

            logger.log(CLogger::LogLevel::Verbose, "Config: user_agent = " + value);
            (*this)["user_agent"] = value;
        }

        else
        {
            logger.log(CLogger::LogLevel::Verbose, "Config: url = " + value);
            (*this)["url"] = value;
        }
    }

    return true;
}

CConfig &CConfig::getInstance()
{
    static CConfig instance;
    return instance;
}

CConfig::Setting &CConfig::operator[](const string &key)
{
    // If key already exists, return setting
    if (m_Settings.find(key) != m_Settings.end())
        return m_Settings.find(key)->second;

    // Otherwise insert new empty setting
    m_Settings.insert(pair<string, Setting>(key, Setting("")));
    return m_Settings.find(key)->second;
}

CConfig::Setting::Setting() = default;

CConfig::Setting::Setting(const string &value)
    : m_Value(value){};

// Get values
CConfig::Setting::operator bool(void) const
{
    return (m_Value == "true") ? true : false;
}

CConfig::Setting::operator int(void) const
{
    return stoi(m_Value);
}

CConfig::Setting::operator string(void) const
{
    return m_Value;
}

// Set values
CConfig::Setting &CConfig::Setting::operator=(bool value)
{
    m_Value = value ? "true" : "false";
    return *this;
}

CConfig::Setting &CConfig::Setting::operator=(int value)
{
    m_Value = to_string(value);
    return *this;
}

CConfig::Setting &CConfig::Setting::operator=(const string &value)
{
    m_Value = value;
    return *this;
}
