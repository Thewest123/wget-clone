/**
 * @file CFile.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Implementation of CFile
 *
 */

#include "CConfig.h"
#include "CFile.h"
#include "CLogger.h"
#include "CHttpsDownloader.h"
#include "CResponse.h"

#include <stdlib.h>
#include <iostream>
#include <filesystem>
#include <sstream>

using std::string, std::ofstream;
namespace fs = std::filesystem;

bool CFile::download()
{
    auto &cfg = CConfig::getInstance();

    // Return if depth exceeded
    if (static_cast<int>(m_Depth) > static_cast<int>(cfg["depth"]))
        return false;

    // Parse path to get m_OutputPath and m_Filename
    parsePath();

    if (fs::exists(m_OutputPath + m_Filename))
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Info, m_Filename + " already exists, skipping!");
        return false;
    }

    CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "Downloading: " + m_Url.getNormURL() + " | (depth " + std::to_string(m_Depth) + ")");

    // Fetch the content from server
    auto response = m_HttpD->get(m_Url);

    // Repeat fetching if the files is moved (301, 302 etc.)
    while (response.getStatus() == CResponse::EStatus::MOVED)
    {
        response = m_HttpD->get(response.m_MovedUrl);
    }

    m_Content = response.getBody();

    // Create folder structure
    fs::create_directories(m_OutputPath);

    save();

    return true;
}

bool CFile::save()
{
    // Write content to a file in the prepared folder structure
    ofstream ofs(m_OutputPath + m_Filename, std::ios_base::out | std::ios_base::binary);
    ofs << m_Content;
    ofs.close();

    return true;
}

void CFile::parsePath()
{
    auto &logger = CLogger::getInstance();
    auto &cfg = CConfig::getInstance();

    string fullPath;

    if (m_Url.isExternal())
    {
        fullPath = m_Url.getDomain() + "/" + m_Url.getNormFilePath();
        m_OutputPath = ((string)cfg["output"]) + "/__external/";
    }
    else
    {
        fullPath = m_Url.getNormFilePath();
        m_OutputPath = ((string)cfg["output"]) + "/";
    }

    m_Filename = fullPath;
    logger.log(CLogger::ELogLevel::Verbose, "getNormFilePath(): " + fullPath);

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

        // Build the output path
        std::stringstream ss;

        ss << ((string)cfg["output"]) << "/";

        if (m_Url.isExternal())
            ss << "__external/";

        ss << fullPath.substr(0, filenameStart + 1);

        // Get only the path without filename
        m_OutputPath = ss.str();
    }

    // If there's no filename, save it as index.html
    if (m_Filename.empty() || m_Filename == "/")
        m_Filename = "index.html";

    logger.log(CLogger::ELogLevel::Verbose, "Path: " + m_OutputPath);
    logger.log(CLogger::ELogLevel::Verbose, "Filename: " + m_Filename);

    return;
}
