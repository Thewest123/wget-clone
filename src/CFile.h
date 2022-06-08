/**
 * @file CFile.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Header file for CFile
 *
 */

#pragma once

#include "CHttpsDownloader.h"
#include "CURLHandler.h"

#include <stdlib.h>
#include <iostream>
#include <filesystem>
#include <set>

#include <memory> // shared_ptr<>
#include <string>

using std::string, std::shared_ptr, std::set;

/**
 * @brief Polymorphic base class to download and store file content, and save it to disk
 *
 */
class CFile
{
public:
    /**
     * @brief Construct a new CFile object
     *
     * @param httpd Pointer to the HttpsDownloader
     * @param depth Current depth of this file
     * @param url URLHandler of this file
     */
    CFile(shared_ptr<CHttpsDownloader> httpd, size_t depth, CURLHandler url)
        : m_HttpD(httpd),
          m_Depth(depth),
          m_Url(url) {}

    /**
     * @brief Process the file, parse content and download other files if needed
     *
     */
    virtual bool download();

    /**
     * @brief Destroy the CFile object
     *
     */
    virtual ~CFile() = default;

protected:
    shared_ptr<CHttpsDownloader> m_HttpD;
    size_t m_Depth;
    CURLHandler m_Url;
    string m_Filename;
    string m_OutputPath;
    string m_Content;

    /**
     * @brief Prepare the required folder structure
     *
     */
    void parsePath();

    /**
     * @brief Flush the File content to a file on disk
     *
     * The m_Filename and m_OutputPath variables need to be set beforehand
     *
     * @return true
     * @return false
     */
    bool save();

    /**
     * @brief Replace external link like "https://google.com/index.html" with relative local link like "../../__external/google.com/index.html"
     *
     */
    void replaceExternalWithLocal(const string &searchString, const CURLHandler &linkUrlHandler);

    /**
     * @brief Get Urls from content with regex pattern
     *
     * @param regexPattern Search pattern
     * @param content Content where to search
     * @return set<string> Set of found URLs
     */
    set<string> getUrlsWithRegex(const string &regexPattern, const string &content);

    /**
     * @brief Create CFile objects from provied URLs
     *
     * @param isExternal True if the current 'urls' set contains external URLs that need different processing
     * @param urls Set of found URLs
     * @param[out] outputFileSet Reference to a set where to insert new CFiles
     */
    void transformUrlsToFiles(bool isExternal, const set<string> &urls, set<shared_ptr<CFile>> &outputFileSet);
};
