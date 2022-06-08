/**
 * @file CFileCss.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Polymorphic derived class that also parses the CSS document and recursively downloads subsequent files
 *
 */

#include <stdlib.h>
#include <iostream>
#include <set>
#include <regex>

#include <memory> // shared_ptr<>
#include <string> // string

#include "CFileCss.h"
#include "CFileHtml.h"
#include "CLogger.h"
#include "CConfig.h"
#include "Utils.h"

// using namespace std;
using std::string, std::stringstream, std::regex, std::regex_replace, std::set, std::cout, std::endl, std::make_shared;

// CFileCss::~CFileCss() = default;

bool CFileCss::download()
{
    // If file already exists or couldn't be downloaded, don't do anything
    if (!CFile::download())
        return false;

    CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "Downloading CSS: " + m_Url.getNormURL() + " | (depth " + std::to_string(m_Depth) + ")");

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

void CFileCss::insertAnnoyingAdvertisementThatNobodyWantsToSee()
{
    if (static_cast<bool>(CConfig::getInstance()["advertisement"]) == false)
        return;

    stringstream ss;
    ss << "\n\n/**\n"
       << " * This page was mirrored using Wget Clone | https://muj.link/wget \n"
       << " *  __    __           _     ___ _                  \n"
       << " * / / /\\ \\ \\__ _  ___| |_  / __\\ | ___  _ __   ___ \n"
       << " * \\ \\/  \\/ / _` |/ _ \\ __|/ /  | |/ _ \\| '_ \\ / _ \\\n"
       << " *  \\  /\\  / (_| |  __/ |_/ /___| | (_) | | | |  __/\n"
       << " *   \\/  \\/ \\__, |\\___|\\__\\____/|_|\\___/|_| |_|\\___|\n"
       << " *          |___/                                   \n"
       << " */\n";

    m_Content += ss.str();

    //    ¯\_(ツ)_/¯
}

void CFileCss::prepareRootUrls()
{
    stringstream replaceString;

    for (size_t i = 0; i < m_Url.getPathDepth(); i++)
    {
        replaceString << "../";
    }

    replaceString << "$1\""; // the link itself

    const regex re_url("(?:url ?\\([\"']?(?!data:|#)((?:\\/)[^;\"'\\s]+)[\"']?)\\)", std::regex_constants::icase);
    const regex re_import("(?:@import [\"']?(?!url\\(|data:|#)((?:\\/)[^;\"'\\s]+)[\"']?)", std::regex_constants::icase);

    m_Content = regex_replace(m_Content, re_url, replaceString.str());
    m_Content = regex_replace(m_Content, re_import, replaceString.str());
}

void CFileCss::replaceExternalWithLocal(const string &searchString, const CURLHandler &linkUrlHandler)
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

set<shared_ptr<CFile>> CFileCss::parseFile()
{
    set<shared_ptr<CFile>> nextFiles;

    set<string> nextUrls;

    try
    {
        // Match everything in url() that doesn't start with http or https
        const regex re_url("(?:url ?\\([\"']?(?!data:|#|http)([^;\"'\\s]+)[\"']?)\\)", std::regex_constants::icase);

        // Match everything in @import that doesn't start with http or https
        const regex re_import("(?:@import [\"']?(?!url\\(|data:|#|http)([^;\"'\\s]+)[\"']?)", std::regex_constants::icase);

        std::sregex_iterator iter_url(m_Content.begin(), m_Content.end(), re_url);
        std::sregex_iterator end_url;

        std::sregex_iterator iter_import(m_Content.begin(), m_Content.end(), re_import);
        std::sregex_iterator end_import;

        while (iter_url != end_url)
        {
            // cout << (*iter)[1] << endl;
            nextUrls.insert((*iter_url)[1]);
            iter_url++;
        }

        while (iter_import != end_import)
        {
            // cout << (*iter)[1] << endl;
            nextUrls.insert((*iter_import)[1]);
            iter_import++;
        }
    }
    catch (std::regex_error &e)
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Error, "Internal error in regex");
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

        CURLHandler newLink(urlNoFilename + i, m_Url.isExternal());

        shared_ptr<CFile> newFile;

        if (Utils::endsWith(newLink.getNormURL(), ".html") || Utils::endsWith(newLink.getNormURL(), ".php") || Utils::endsWith(newLink.getNormURL(), "/"))
            newFile = make_shared<CFileHtml>(m_HttpD, m_Depth, newLink);
        else if (Utils::endsWith(newLink.getNormURL(), ".css"))
            newFile = make_shared<CFileCss>(m_HttpD, m_Depth, newLink);
        else
            newFile = make_shared<CFile>(m_HttpD, m_Depth, newLink);

        nextFiles.insert(newFile);
        CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "Next file: " + newLink.getNormURL() + " | (depth " + std::to_string(m_Depth) + ")");
    }

    // ----------- External links ---------------

    // Skip external links if desired
    if (static_cast<bool>(CConfig::getInstance()["remote"]) == true)
        return nextFiles;

    set<string> nextUrlsExternal;

    try
    {
        // Match everything in url() that DOES start with http or https
        const regex re_url("(?:url ?\\([\"']?(?!data:|#)((?:http:\\/\\/|https:\\/\\/)[^;\"'\\s]+)[\"']?)\\)", std::regex_constants::icase);

        // Match everything in @import that DOES start with http or https
        const regex re_import("(?:@import [\"']?(?!url\\(|data:|#)((?:http:\\/\\/|https:\\/\\/)[^;\"'\\s]+)[\"']?)", std::regex_constants::icase);

        std::sregex_iterator iter_url(m_Content.begin(), m_Content.end(), re_url);
        std::sregex_iterator end_url;

        std::sregex_iterator iter_import(m_Content.begin(), m_Content.end(), re_import);
        std::sregex_iterator end_import;

        while (iter_url != end_url)
        {
            // cout << (*iter)[1] << endl;
            nextUrls.insert((*iter_url)[1]);
            iter_url++;
        }

        while (iter_import != end_import)
        {
            // cout << (*iter)[1] << endl;
            nextUrls.insert((*iter_import)[1]);
            iter_import++;
        }
    }
    catch (std::regex_error &e)
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Error, "Internal error in regex");
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
            CLogger::getInstance().log(CLogger::ELogLevel::Info, "Skipping link due to limit: " + newLink.getNormURL());
            continue;
        }

        shared_ptr<CFile> newFile;

        if (Utils::endsWith(newLink.getNormURL(), ".html") || Utils::endsWith(newLink.getNormURL(), ".php") || Utils::endsWith(newLink.getNormURL(), "/"))
            newFile = make_shared<CFileHtml>(m_HttpD, m_Depth + 1, newLink);
        else if (Utils::endsWith(newLink.getNormURL(), ".css"))
            newFile = make_shared<CFileCss>(m_HttpD, m_Depth + 1, newLink);
        else
            newFile = make_shared<CFile>(m_HttpD, m_Depth + 1, newLink);

        nextFiles.insert(newFile);
        CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "Next EXTERNAL file: " + newLink.getNormURL() + " | (depth " + std::to_string(m_Depth + 1) + ")");

        if (static_cast<int>(m_Depth) + 1 <= static_cast<int>(CConfig::getInstance()["depth"]))
            replaceExternalWithLocal(i, newLink);
    }

    return nextFiles;
}
