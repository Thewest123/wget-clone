/**
 * @file CFile.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Polymorphic base class to download and store file content, and save it to disk
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

using std::string, std::ofstream;
namespace fs = std::filesystem;

bool CFile::download()
{
    auto &cfg = CConfig::getInstance();

    if (static_cast<int>(m_Depth) > static_cast<int>(cfg["depth"]))
    {
        // if (static_cast<bool>(cfg["error_page"]))
        // {
        //     parsePath();

        //     if (fs::exists(m_OutputPath + m_Filename))
        //         return false;

        //     // Copy 404.html file into this file
        //     std::ifstream errorFile("../assets/404.html", std::ios::binary);
        //     std::ofstream saveFile(m_OutputPath + m_Filename, std::ios::binary);

        //     saveFile << errorFile.rdbuf();
        //     std::cout << "saved as 404.html" << std::endl;
        // }

        return false;
    }

    // Parse path to get m_OutputPath and m_Filename
    parsePath();

    if (fs::exists(m_OutputPath + m_Filename))
    {
        CLogger::getInstance().log(CLogger::LogLevel::Info, m_Filename + " already exists, skipping!");
        return false;
    }

    CLogger::getInstance().log(CLogger::LogLevel::Verbose, "Downloading: " + m_Url.getNormURL() + " | (depth " + std::to_string(m_Depth) + ")");

    // Fetch the content from server
    auto response = m_HttpD->get(m_Url);
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
        m_OutputPath = ((string)cfg["output"]) + "/__external/";

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
            m_OutputPath = ((string)cfg["output"]) + "/__external/" + fullPath.substr(0, filenameStart + 1);
        }

        // If there's no filename, save it as index.html
        if (m_Filename.empty() || m_Filename == "/")
            m_Filename = "index.html";

        logger.log(CLogger::LogLevel::Verbose, "Path: " + m_OutputPath);
        logger.log(CLogger::LogLevel::Verbose, "Filename: " + m_Filename);
    }

    return;
}
