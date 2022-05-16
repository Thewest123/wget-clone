/**
 * @file CFile.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <stdlib.h>
#include <iostream>

#include "CHttpDownloader.h"

using namespace std;

class CFile
{
public:
    CFile(size_t depth, const string &url)
        : m_Depth(depth),
          m_Url(url){};

    /**
     * @brief Fetch the File from URL and save it to disk
     *
     */
    virtual void download()
    {
    }

private:
    size_t m_Depth;
    string m_Url;
};