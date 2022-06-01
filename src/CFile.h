#pragma once

/**
 * @file CFile.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <stdlib.h>
#include <iostream>
#include <filesystem>

#include "CHttpsDownloader.h"
#include "CConfig.h"
#include "CURLHandler.h"

using namespace std;

class CFile
{
public:
    CFile(shared_ptr<CHttpsDownloader> httpd, size_t depth, CURLHandler url)
        : m_HttpD(httpd),
          m_Depth(depth),
          m_Url(url){};

    /**
     * @brief Fetch the File from URL and save it to disk
     *
     */
    virtual bool download();

    bool save();

    virtual ~CFile() = default;

protected:
    shared_ptr<CHttpsDownloader> m_HttpD;
    size_t m_Depth;
    CURLHandler m_Url;
    // string m_Host;
    string m_Filename;
    // string m_Path;
    string m_OutputPath;
    string m_Content;
    // bool m_IsExternal;

    void createPath();
};