#pragma once

/**
 * @file CFile.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <stdlib.h>
#include <iostream>
#include <set>
#include <regex>

#include "CFile.h"
#include "CHttpsDownloader.h"
#include "CURLHandler.h"

using namespace std;

class CFileHtml : public CFile
{
public:
    CFileHtml(shared_ptr<CHttpsDownloader> httpd, size_t depth, CURLHandler url)
        : CFile(httpd, depth, url){};

    virtual ~CFileHtml() = default;

    /**
     * @brief Fetch the File from URL and save it to disk, recursively download other files
     *
     */
    virtual bool download() override;

private:
    /**
     * @brief Parse the file and return other files to download
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
     * @brief Insert ASCII art with project link to the end of Html file
     *
     */
    void insertAnnoyingAdvertisementThatNobodyWantsToSee();
};