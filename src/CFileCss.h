/**
 * @file CFileCss.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
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

class CFileCss : public CFile
{
public:
    /**
     * @brief Construct a new CFileCss object
     *
     * @param httpd Pointer to the HttpsDownloader
     * @param depth Current depth of this file
     * @param url URLHandler of this file
     */
    CFileCss(shared_ptr<CHttpsDownloader> httpd, size_t depth, CURLHandler url)
        : CFile(httpd, depth, url) {}

    /**
     * @brief Destroy the CFileCss object
     *
     */
    virtual ~CFileCss() = default;

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
     * @brief Preproccess the Css source code - replace absolute paths with relative paths
     *
     * For example: url("/assets/main.css") may become url("../../assets/main.css")
     *
     */
    void prepareRootUrls();

    /**
     * @brief Replace external link like "https://google.com/index.html" with relative local link like "../../__external/google.com/index.html"
     *
     */
    void replaceExternalWithLocal(const string &searchString, const CURLHandler &linkUrlHandler);

    /**
     * @brief Insert ASCII art with project link to the end of Css file
     *
     */
    void insertAnnoyingAdvertisementThatNobodyWantsToSee();
};
