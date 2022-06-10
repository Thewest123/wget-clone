/**
 * @file CURLHandler.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Header file for CURLHandler
 */

#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm> // transform
#include <regex>
#include <iostream>

// using namespace std;
using std::string, std::vector;

/**
 * @brief URL Handler to parse various URLs, append relative paths to existings URLs, and to provide normalized URLs and file paths
 *
 */
class CURLHandler
{
public:
    CURLHandler() = default;

    /**
     * @brief Construct a new CURLHandler object
     *
     * @param url Base URL of the CURLHandler, can be just domain or full URL
     */
    CURLHandler(const string &url, bool isExternal = false);

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

    /**
     * @brief Get the Domain normalized (remove www. if present)
     *
     * @return string Normalized Domain
     */
    string getDomainNorm() const;

    /**
     * @brief Returns bool is current URL uses https protocol
     *
     * @return true Https enabled
     * @return false Https disabled or protocol unknown
     */
    bool isHttps() const;

    /**
     * @brief Get depth of the current URL based on m_PathLevels
     *
     * @return size_t
     */
    size_t getPathDepth() const;

    /**
     * @brief Returns bool if the current URL is marked as an external
     *
     * @return true Is external
     * @return false Is NOT external
     */
    bool isExternal() const;

    /**
     * @brief Set current URL to (not)external
     *
     * @param isExternal
     */
    void setExternal(bool isExternal);

private:
    bool m_IsHttps = false;
    bool m_HasTrailingSlash = true;
    bool m_IsExternal = false;
    string m_Domain;

    /**
     * @brief Vector of strings containing levels of path
     *
     * eg. URL path "../directory/file.html" is split into "..", "directory", "file.html"
     *
     */
    vector<string> m_PathLevels;

    /**
     * @brief Returns m_PathLevels normalized, eg. won't include '../' etc.
     *
     * @return vector<string> Normalized m_PathLevels
     */
    vector<string> getNormalizedLevels() const;
};
