#pragma once

/**
 * @file CURLHandler.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief
 * @version 0.1
 * @date 2022-05-27
 *
 * @copyright Copyright Jan Cerny (c) 2022
 *
 */

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
    /**
     * @brief Construct a new CURLHandler object
     *
     * @param url Base URL of the CURLHandler, can be just domain or full URL
     */
    CURLHandler(const string &url, bool isExternal = false);

    ~CURLHandler();

    /**
     * @brief Sets the domain URL and deducts protocol from 'urlDomain'
     *
     * @param urlDomain Domain of the URL, eg. https://google.com/
     */
    void setDomain(const string &urlDomain);

    /**
     * @brief Add additional relative part of path to the existing URL
     *
     * @param path Additional relative path (eg. "next/directory/../index.html")
     */
    void addPath(const string &path);

    /**
     * @brief Returns the full normalized URL with fixed path changes (eg. '../')
     *
     * @return string Normalized URL
     */
    string getNormURL() const;

    /**
     * @brief Like getNormFilePath(), returns only the normalized path without domain, but includes trailing slash if suitable
     *
     * @return string Normalized path only, including trailing slash if suitable
     */
    string getNormURLPath() const;

    /**
     * @brief Returns only the normalized path without domain, with fixed path changes (eg. '../')
     *
     * @return string Normalized path only
     */
    string getNormFilePath() const;

    /**
     * @brief Get the Domain
     *
     * @return string Domain
     */
    string getDomain() const;

    string getDomainNorm() const;

    /**
     * @brief Returns bool is current URL uses https protocol
     *
     * @return true Https enabled
     * @return false Https disabled or protocol unknown
     */
    bool isHttps() const;

    size_t getPathDepth() const;

    bool isExternal() const;

private:
    bool m_IsHttps = false;
    bool m_HasTrailingSlash = true;
    string m_Domain;
    vector<string> m_PathLevels;
    bool m_IsExternal = false;
    vector<string> getNormalizedLevels() const;
};