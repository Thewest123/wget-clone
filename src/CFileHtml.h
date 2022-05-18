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
#include "CHttpDownloader.h"

using namespace std;

class CFileHtml : public CFile
{
public:
    CFileHtml(shared_ptr<CHttpDownloader> httpd, size_t depth, const string &url)
        : CFile(httpd, depth, url){};

    virtual ~CFileHtml() = default;

    /**
     * @brief Fetch the File from URL and save it to disk, recursively download other files
     *
     */
    virtual void download() override;

private:
    /**
     * @brief Parse the file and return other files to download
     *
     * @return set<CFile>
     */
    set<shared_ptr<CFile>> parseFile();
    bool ends_with(std::string const & value, std::string const & ending);

};