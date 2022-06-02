/**
 * @file CFile.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <stdlib.h>
#include <iostream>
#include <filesystem>

#include "CHttpsDownloader.h"
#include "CConfig.h"
#include "CFile.h"
#include "CLogger.h"

using namespace std;

// CFile::~CFile() = default;

bool CFile::download()
{
    auto &cfg = CConfig::getInstance();

    if ((int)m_Depth > (int)cfg["depth"])
        return false;

    // Parse path and create folder structure
    createPath();

    if (filesystem::exists(m_OutputPath + m_Filename))
    {
        CLogger::getInstance().log(CLogger::LogLevel::Info, m_Filename + " already exists, skipping!");
        return false;
    }

    CLogger::getInstance().log(CLogger::LogLevel::Verbose, "Downloading: " + m_Url.getNormURL() + " | (depth " + to_string(m_Depth) + ")");

    // Fetch the content from server
    m_Content = m_HttpD->get(m_Url);

    createPath();

    save();

    return true;
}

bool CFile::save()
{
    // Write content to a file in the prepared folder structure
    ofstream ofs(m_OutputPath + m_Filename, ios_base::out | ios_base::binary);
    ofs << m_Content;

    return true;
}

void CFile::createPath()
{
    // ! FIXME: sloucit if else dohromady, duplicitni kod
    if (!m_Url.isExternal())
    {
        auto &logger = CLogger::getInstance();
        auto &cfg = CConfig::getInstance();

        string fullPath = m_Url.getNormFilePath();
        logger.log(CLogger::LogLevel::Verbose, "getNormFilePath(): " + fullPath);

        m_Filename = fullPath;
        m_OutputPath = ((string)cfg["output"]) + "/";

        // Find position of the last slash
        size_t filenameStart = fullPath.find_last_of('/');

        // If it exists and it's not the last slash (meaning theres a filename at the end, eg. index.html)
        if (filenameStart != string::npos && filenameStart != fullPath.length())
        {
            // Get only the filename
            m_Filename = fullPath.substr(filenameStart + 1);

            // Remove query params (everything after '?') from filename
            size_t filenameEndPos = string::npos;

            if ((filenameEndPos = m_Filename.find('?')) != string::npos)
                m_Filename = m_Filename.substr(0, filenameEndPos);

            // Get only the path without filename
            m_OutputPath = ((string)cfg["output"]) + "/" + fullPath.substr(0, filenameStart + 1);
        }

        // If there's no filename, save it as index.html
        if (m_Filename.empty() || m_Filename == "/")
            m_Filename = "index.html";

        // Create folder structure
        filesystem::create_directories(m_OutputPath);

        logger.log(CLogger::LogLevel::Verbose, "Path: " + m_OutputPath);
        logger.log(CLogger::LogLevel::Verbose, "Filename: " + m_Filename);
    }
    else
    {
        auto &logger = CLogger::getInstance();
        auto &cfg = CConfig::getInstance();

        string fullPath = m_Url.getDomain() + "/" + m_Url.getNormFilePath();
        logger.log(CLogger::LogLevel::Verbose, "getNormFilePath(): " + fullPath);

        m_Filename = fullPath;
        m_OutputPath = ((string)cfg["output"]) + "/external/";

        // Find position of the last slash
        size_t filenameStart = fullPath.find_last_of('/');

        // If it exists and it's not the last slash (meaning theres a filename at the end, eg. index.html)
        if (filenameStart != string::npos && filenameStart != fullPath.length())
        {
            // Get only the filename
            m_Filename = fullPath.substr(filenameStart + 1);

            // Remove query params (everything after '?') from filename
            size_t filenameEndPos = string::npos;

            if ((filenameEndPos = m_Filename.find('?')) != string::npos)
                m_Filename = m_Filename.substr(0, filenameEndPos);

            // Get only the path without filename
            m_OutputPath = ((string)cfg["output"]) + "/external/" + fullPath.substr(0, filenameStart + 1);
        }

        // If there's no filename, save it as index.html
        if (m_Filename.empty() || m_Filename == "/")
            m_Filename = "index.html";

        // Create folder structure
        filesystem::create_directories(m_OutputPath);

        logger.log(CLogger::LogLevel::Verbose, "Path: " + m_OutputPath);
        logger.log(CLogger::LogLevel::Verbose, "Filename: " + m_Filename);
    }

    return;
}
