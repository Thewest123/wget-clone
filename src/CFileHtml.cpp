/**
 * @file CFileHtml.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Polymorphic derived class that also parses the HTML document and recursively downloads subsequent files
 *
 */

#include <stdlib.h>
#include <iostream>
#include <set>
#include <regex>

#include <memory> // shared_ptr<>
#include <string> // string

#include "CFileHtml.h"
#include "CLogger.h"
#include "CConfig.h"
#include "Utils.h"

// using namespace std;
using std::string, std::stringstream, std::regex, std::regex_replace, std::set, std::cout, std::endl, std::make_shared;

// CFileHtml::~CFileHtml() = default;

bool CFileHtml::download()
{
    // If file already exists or couldn't be downloaded, don't do anything
    if (!CFile::download())
        return false;

    CLogger::getInstance().log(CLogger::LogLevel::Verbose, "Downloading HTML: " + m_Url.getNormURL() + " | (depth " + std::to_string(m_Depth) + ")");

    prepareRootUrls();

    auto newFiles = parseFile();

    insertAnnoyingAdvertisementThatNobodyWantsToSee();

    save();

    for (auto &&i : newFiles)
    {
        i->download();
    }

    return true;
}

void CFileHtml::insertAnnoyingAdvertisementThatNobodyWantsToSee()
{
    if (static_cast<bool>(CConfig::getInstance()["advertisement"]) == false)
        return;

    stringstream ss;
    ss << "\n\n<!--\n";
    ss << " This page was mirrored using Wget Clone | https://muj.link/wget \n";
    ss << " __    __           _     ___ _                  \n";
    ss << "/ / /\\ \\ \\__ _  ___| |_  / __\\ | ___  _ __   ___ \n";
    ss << "\\ \\/  \\/ / _` |/ _ \\ __|/ /  | |/ _ \\| '_ \\ / _ \\\n";
    ss << " \\  /\\  / (_| |  __/ |_/ /___| | (_) | | | |  __/\n";
    ss << "  \\/  \\/ \\__, |\\___|\\__\\____/|_|\\___/|_| |_|\\___|\n";
    ss << "         |___/                                   \n";
    ss << "-->\n";

    m_Content += ss.str();

    //    ¯\_(ツ)_/¯
}

void CFileHtml::prepareRootUrls()
{
    stringstream replaceString;

    replaceString << "$1\""; // src= or href=

    for (size_t i = 0; i < m_Url.getPathDepth(); i++)
    {
        replaceString << "../";
    }

    replaceString << "$2\""; // the link itself

    const regex re("(srcset=|src=|href=)[\"']\\/([^\\/][^\"']*)[\"']", std::regex_constants::icase);
    m_Content = regex_replace(m_Content, re, replaceString.str());
}

void CFileHtml::replaceExternalWithLocal(const string &searchString, const CURLHandler &linkUrlHandler)
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

    cout << "Replaced '" << searchString << "' with '" << replaceString.str() << "'" << endl;
}

set<shared_ptr<CFile>> CFileHtml::parseFile()
{
    set<shared_ptr<CFile>> nextFiles;

    set<string> nextUrls;

    try
    {
        // Match everything in src= and href= that doesn't start with http or https
        const regex re("(?:src=|href=)[\"'](?!http:\\/\\/|https:\\/\\/|data:|//)([^\"'#]*)(#?[^\"']*)[\"']", std::regex_constants::icase);

        std::sregex_iterator iter(m_Content.begin(), m_Content.end(), re);
        std::sregex_iterator end;

        while (iter != end)
        {
            // cout << (*iter)[1] << endl;
            nextUrls.insert((*iter)[1]);
            iter++;
        }
    }
    catch (std::regex_error &e)
    {
        CLogger::getInstance().log(CLogger::LogLevel::Error, "Internal error in regex");
    }

    for (auto &&i : nextUrls)
    {
        string urlNoFilename = m_Url.getDomain();

        // If next URL starts with a slash, it's relative to the root domain, no need to get previous path
        if (!Utils::startsWith(i, "/"))
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

        CURLHandler newLink(urlNoFilename + i);

        shared_ptr<CFile> newFile;

        if (Utils::endsWith(newLink.getNormURL(), ".html") || Utils::endsWith(newLink.getNormURL(), ".php") || Utils::endsWith(newLink.getNormURL(), "/"))
            newFile = make_shared<CFileHtml>(m_HttpD, m_Depth + 1, newLink);
        else
            newFile = make_shared<CFile>(m_HttpD, m_Depth + 1, newLink);

        nextFiles.insert(newFile);
        CLogger::getInstance().log(CLogger::LogLevel::Verbose, "Next file: " + newLink.getNormURL() + " | (depth " + std::to_string(m_Depth + 1) + ")");
    }

    // ----------- External links ---------------

    // Skip external links if desired
    if (static_cast<bool>(CConfig::getInstance()["remote"]) == true)
        return nextFiles;

    set<string> nextUrlsExternal;

    try
    {
        // Match everything in src= and href= that DOES start with http or https
        const regex re_external("(?:src=|href=)[\"']((?:http:\\/\\/|https:\\/\\/)[^\"'#]*)(#?[^\"']*)[\"']", std::regex_constants::icase);

        std::sregex_iterator iter_external(m_Content.begin(), m_Content.end(), re_external);
        std::sregex_iterator end_external;

        while (iter_external != end_external)
        {
            nextUrlsExternal.insert((*iter_external)[1]);
            iter_external++;
        }
    }
    catch (std::regex_error &e)
    {
        CLogger::getInstance().log(CLogger::LogLevel::Error, "Internal error in regex");
    }

    for (auto &&i : nextUrlsExternal)
    {
        CURLHandler newLink(i, true);

        // Skip if we are referencing ourselves
        CURLHandler mainPageUrl(static_cast<string>(CConfig::getInstance()["url"]));
        if (newLink.getDomain() == mainPageUrl.getDomain())
            continue;

        // Skip if URL is not in limited links, if specified
        string domainsList = static_cast<string>(CConfig::getInstance()["limit"]);

        if (!domainsList.empty() && !Utils::contains(domainsList, newLink.getDomain()))
        {
            CLogger::getInstance().log(CLogger::LogLevel::Info, "Skipping link due to limit: " + newLink.getNormURL());
            continue;
        }

        shared_ptr<CFile> newFile;

        if (Utils::endsWith(newLink.getNormURL(), ".html") || Utils::endsWith(newLink.getNormURL(), ".php") || Utils::endsWith(newLink.getNormURL(), "/"))
            newFile = make_shared<CFileHtml>(m_HttpD, m_Depth + 1, newLink);
        else
            newFile = make_shared<CFile>(m_HttpD, m_Depth + 1, newLink);

        nextFiles.insert(newFile);
        CLogger::getInstance().log(CLogger::LogLevel::Verbose, "Next EXTERNAL file: " + newLink.getNormURL() + " | (depth " + std::to_string(m_Depth + 1) + ")");

        if (static_cast<int>(m_Depth) + 1 <= static_cast<int>(CConfig::getInstance()["depth"]))
            replaceExternalWithLocal(i, newLink);
    }

    return nextFiles;
}
