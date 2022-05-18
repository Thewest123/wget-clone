#pragma once

/**
 * @file CConfig.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <string>
#include <map>

using namespace std;

class CConfig
{
public:

    struct Setting
    {
        Setting();
        Setting(const string &value);

        string m_Value;

        // Get values
        operator bool() const;
        operator int() const;
        operator string() const;

        // Set values
        Setting &operator= (bool);
        Setting &operator= (int);
        Setting &operator= (const string &);
    };

    CConfig()
    {
        (*this)["test"] = false;
    };
    
    CConfig::Setting &operator[] (const string &);

    // Singleton stuff
    static CConfig &getInstance();

    CConfig(const CConfig &)  = delete;
    void operator=(const CConfig &) = delete;


private:

    map<string, Setting> m_Settings;

};