#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm> // transform
#include <regex>
#include <iostream>
#include "Utils.h"

using namespace std;

class CURLHandler
{
public:
    CURLHandler(const string &url);
    ~CURLHandler();
    void setDomain(const string &urlDomain);
    void addPath(const string &path);
    string getNormURL() const;
    string getNormURLPath() const;
    string getNormFilePath() const;
    string getDomain() const;
    bool isHttps() const;

private:
    // string toLowerCase(const string &str) const;
    // bool endsWith(const string &str, const string &ending) const;
    bool m_IsHttps = false;
    bool m_HasTrailingSlash = true;
    string m_Domain;
    vector<string> m_PathLevels;
};