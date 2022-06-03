/**
 * @file CFileCss.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#pragma once

#include "CFile.h"

#include <stdlib.h>

#include <iostream>
#include <string>
#include <set>

// using namespace std;
using std::string, std::set;

class CFileCss : CFile
{
public:
    CFileCss(shared_ptr<CHttpsDownloader> httpd, size_t depth, CURLHandler url)
        : CFile(httpd, depth, url) {}

    /**
     * @brief Fetch the File from URL and save it to disk, recursively download other files
     *
     */
    virtual bool download() override
    {
    }

private:
    /**
     * @brief Parse the file and return other files to download
     *
     * @return set<CFile>
     */
    set<CFile> parseFile()
    {
    }
};
