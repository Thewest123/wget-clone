/**
 * @file CConfig.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include "CConfig.h"

using namespace std;

CConfig &CConfig::getInstance()
{
    static CConfig instance;
    return instance;
}

CConfig::Setting &CConfig::operator[] (const string &key)
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
    : m_Value(value) {};

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
CConfig::Setting& CConfig::Setting::operator= (bool value)
{
    m_Value = value ? "true" : "false";
    return *this;
}

CConfig::Setting& CConfig::Setting::operator= (int value)
{
    m_Value = to_string(value);
    return *this;
}

CConfig::Setting& CConfig::Setting::operator= (const string &value)
{
    m_Value = value;
    return *this;
}

