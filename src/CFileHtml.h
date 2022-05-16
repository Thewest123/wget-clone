/**
 * @file CFile.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <stdlib.h>
#include <iostream>
#include <set>

#include "CFile.h"
#include "CHttpDownloader.h"

using namespace std;

class CFileHtml : CFile
{
public:
    CFileHtml(size_t depth, const string &url)
        : CFile(depth, url){};

    /**
     * @brief Fetch the File from URL and save it to disk, recursively download other files
     *
     */
    virtual void download() override
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