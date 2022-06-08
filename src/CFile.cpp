/**
 * @file CFile.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Implementation of CFile
 *
 */

#include "CConfig.h"
#include "CFile.h"
#include "CFileHtml.h"
#include "CFileCss.h"
#include "CLogger.h"
#include "CResponse.h"
#include "Utils.h"

#include <stdlib.h>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <regex>

using std::string, std::ofstream, std::regex, std::make_shared, std::stringstream;
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

    CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "Processing: " + m_Url.getNormURL() + " | (depth " + std::to_string(m_Depth) + ")");

    // Fetch the content from server
    auto response = m_HttpD->get(m_Url);

    // Repeat fetching if the files is moved (301, 302 etc.)
    while (response.m_Status == CResponse::EStatus::MOVED)
    {
        response = m_HttpD->get(response.m_MovedUrl);
    }

    m_Content = response.m_Body;

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

set<string> CFile::getUrlsWithRegex(const string &regexPattern, const string &content)
{
    set<string> foundUrls;

    try
    {
        const regex re(regexPattern, std::regex_constants::icase);

        std::sregex_iterator iter(content.begin(), content.end(), re);
        std::sregex_iterator end;

        while (iter != end)
        {
            string value = (*iter)[1];
            iter++;
            foundUrls.emplace(value);
        }
    }
    catch (std::regex_error &e)
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Error, "Internal error in regex");
    }

    return foundUrls;
}

void CFile::transformUrlsToFiles(bool isExternal, const set<string> &urls, set<shared_ptr<CFile>> &outputFileSet)
{
    for (const auto &url : urls)
    {
        CURLHandler newLink;

        if (isExternal)
        {
            newLink = CURLHandler(url, true);

            // Skip if we are referencing ourselves
            CURLHandler mainPageUrl(static_cast<string>(CConfig::getInstance()["url"]));
            if (newLink.getDomain() == mainPageUrl.getDomain())
                continue;

            // Skip if URL is not in limited links, if specified
            string domainsList = static_cast<string>(CConfig::getInstance()["limit"]);

            if (!domainsList.empty() && !Utils::contains(domainsList, newLink.getDomain()))
            {
                CLogger::getInstance().log(CLogger::ELogLevel::Info, "Skipping link due to limit: " + newLink.getNormURL());
                continue;
            }
        }
        else
        {
            string urlNoFilename = m_Url.getDomain();

            // If next URL starts with a slash, it's relative to the root domain, no need to get previous path
            if (!Utils::startsWith(url, "/"))
            {
                urlNoFilename = m_Url.getNormURL();

                // Find position of the last slash
                size_t filenameStart = urlNoFilename.find_last_of('/');

                // If it exists and it's not the last slash (meaning theres a filename at the end, eg. index.html)
                if (filenameStart != string::npos && filenameStart != urlNoFilename.length())
                {
                    // Get only the path without filename
                    urlNoFilename = urlNoFilename.substr(0, filenameStart + 1);
                }
            }

            newLink = CURLHandler(urlNoFilename + url, m_Url.isExternal());
        }

        shared_ptr<CFile> newFile;

        if (Utils::endsWith(newLink.getNormURL(), ".html") || Utils::endsWith(newLink.getNormURL(), ".php") || Utils::endsWith(newLink.getNormURL(), "/"))
            newFile = make_shared<CFileHtml>(m_HttpD, m_Depth + 1, newLink);
        else if (Utils::endsWith(newLink.getNormURL(), ".css"))
            newFile = make_shared<CFileCss>(m_HttpD, m_Depth, newLink);
        else
            newFile = make_shared<CFile>(m_HttpD, m_Depth + 1, newLink);

        outputFileSet.insert(newFile);

        string logOutput = isExternal ? "Next external file: " : "Next relative file:";
        CLogger::getInstance().log(CLogger::ELogLevel::Verbose, logOutput + newLink.getNormURL() + " | (depth " + std::to_string(m_Depth + 1) + ")");

        if (isExternal &&
            static_cast<int>(m_Depth) + 1 <= static_cast<int>(CConfig::getInstance()["depth"]))
            replaceExternalWithLocal(url, newLink);
    }
}

void CFile::replaceExternalWithLocal(const string &searchString, const CURLHandler &linkUrlHandler)
{
    stringstream replaceString;

    // Get relative path to root directory
    for (size_t i = 0; i < m_Url.getPathDepth(); i++)
    {
        replaceString << "../";
    }

    string pathWithFixedFilename = linkUrlHandler.getNormFilePath();

    // Remove query params (everything after '?') from filename
    size_t filenameEndPos = string::npos;

    if ((filenameEndPos = pathWithFixedFilename.find('?')) != string::npos)
        pathWithFixedFilename = pathWithFixedFilename.substr(0, filenameEndPos);

    // Get path to local external directory
    replaceString << "__external/"
                  << linkUrlHandler.getDomain()
                  << "/"
                  << pathWithFixedFilename;

    Utils::replaceAll(m_Content, searchString, replaceString.str());

    CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "Replaced all '" + searchString + "' with '" + replaceString.str() + "'");
}