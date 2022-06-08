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

#include <memory> // shared_ptr<>
#include <string>

using std::string, std::shared_ptr;

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
     * @brief Fetch the File from URL and save it to disk
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
};
