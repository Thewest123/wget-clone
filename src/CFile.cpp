/**
 * @file CFile.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <stdlib.h>
#include <iostream>
#include <filesystem>

#include "CHttpDownloader.h"
#include "CConfig.h"
#include "CFile.h"
#include "CLogger.h"

using namespace std;

// CFile::~CFile() = default;

void CFile::download()
{

    m_Content = m_HttpD->get(m_Url);

    createPath();

    auto &logger = CLogger::getInstance();
    logger.log(CLogger::LogLevel::Verbose, "ofs: " + m_OutputPath + m_Filename);

    ofstream ofs(m_OutputPath + m_Filename, ios_base::out | ios_base::binary);
    ofs << m_Content;
}

void CFile::createPath()
{
    const regex re("^(?:http:\\/\\/|https:\\/\\/)?((?:www\\.)?[^/]*)([^\\.?]*)(\\/[^?]*).*", regex_constants::icase);
    smatch result;

    auto &logger = CLogger::getInstance();
    logger.log(CLogger::LogLevel::Verbose, "ofs: " + m_OutputPath + m_Filename);

    if (regex_match(m_Url, result, re) != true || result.size() < 2)
    {
        logger.log(CLogger::LogLevel::Error, "Error matching path");
        return;
    }

    m_Host = result[1].str();
    m_Path = result[2].str();
    m_Filename = result[3].str();

    if (m_Filename.empty() || m_Filename == "/")
        m_Filename = "/index.html";

    m_OutputPath = "./output" + m_Path;
    
    logger.log(CLogger::LogLevel::Verbose, "Path: " + m_Path);
    logger.log(CLogger::LogLevel::Verbose, "Filename: " + m_Filename);


    filesystem::create_directories(m_OutputPath);

    return;
}
