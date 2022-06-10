/**
 * @file CFileHtml.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Header file for CFileHtml
 *
 */

#pragma once

#include "CFile.h"
#include "CURLHandler.h"

#include <stdlib.h>
#include <iostream>
#include <set>
#include <regex>
#include <memory> // shared_ptr<>
#include <string>

using std::set, std::shared_ptr, std::string;

/**
 * @brief Polymorphic derived class that also parses the HTML document and recursively downloads subsequent files
 *
 */
class CFileHtml : public CFile
{
public:
    /**
     * @brief Construct a new CFileHtml object
     *
     * @param httpd Pointer to the HttpsDownloader
     * @param depth Current depth of this file
     * @param url URLHandler of this file
     */
    CFileHtml(shared_ptr<CHttpsDownloader> httpd, size_t depth, CURLHandler url)
        : CFile(httpd, depth, url) {}

    /**
     * @brief Destroy the CFileHtml object
     *
     */
    virtual ~CFileHtml() = default;

    /**
     * @brief Fetch the File from URL and save it to disk, recursively download subsequent files
     *
     */
    virtual bool download() override;

private:
    /**
     * @brief Parse the file and return subsequent files to download
     *
     * @return set<CFile>
     */
    set<shared_ptr<CFile>> parseFile();

    /**
     * @brief Preproccess the Html source code - replace absolute paths with relative paths
     *
     * For example: src="/assets/main.js" may become src="../../assets/main.js"
     *
     */
    void prepareRootUrls();

    /**
     * @brief Replace all relative local links in images to external links
     *
     * For example: <img src="assets/main.js"> may become <img src="https://google.com/assets/main.js"
     *
     */
    void makeRelativeImagesExternal();

    /**
     * @brief Insert ASCII art with project link to the end of Html file
     *
     */
    void insertAnnoyingAdvertisementThatNobodyWantsToSee();
};
