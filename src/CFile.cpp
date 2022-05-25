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

void CFile::download()
{
    auto &cfg = CConfig::getInstance();

    if ((int)m_Depth > (int)cfg["depth"])
        return;

    // Fetch the content from server
    m_Content = m_HttpD->get(m_Url);

    // Parse path and create folder structure
    createPath();

    // Write content to a file in the prepared folder structure
    ofstream ofs(m_OutputPath + m_Filename, ios_base::out | ios_base::binary);
    ofs << m_Content;
}

void CFile::createPath()
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

    return;
}
