/**
 * @file CFileHtml.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Implementation of CFileHtml
 *
 */

#include <stdlib.h>
#include <iostream>
#include <set>
#include <regex>
#include <filesystem>
#include <fstream>

#include <memory> // shared_ptr<>
#include <string>

#include "CFileHtml.h"
#include "CFileCss.h"
#include "CLogger.h"
#include "CConfig.h"
#include "Utils.h"

using std::string, std::stringstream, std::regex, std::regex_replace, std::set, std::endl, std::make_shared, std::ifstream, std::ofstream;
namespace fs = std::filesystem;

// CFileHtml::~CFileHtml() = default;

bool CFileHtml::download()
{
    // If file already exists or couldn't be downloaded, don't do anything
    if (!CFile::download())
    {
        auto &cfg = CConfig::getInstance();

        // If download failed because of depth, insert error page
        if (static_cast<int>(m_Depth) > static_cast<int>(cfg["depth"]) &&
            static_cast<bool>(cfg["error_page"]))
        {
            parsePath();

            if (fs::exists(m_OutputPath + m_Filename))
                return false;

            // Create folder structure
            fs::create_directories(m_OutputPath);

            // Copy 404.html file into this file
            ifstream errorFile("./assets/404.html", std::ios::binary | std::ios::in);
            if (errorFile.fail())
                throw std::runtime_error("Cannot open asset 404.html!");

            ofstream saveFile(m_OutputPath + m_Filename, std::ios::binary | std::ios::out);

            saveFile << errorFile.rdbuf();
            saveFile.close();
        }

        return false;
    }

    CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "Downloading HTML: " + m_Url.getNormURL() + " | (depth " + std::to_string(m_Depth) + ")");

    prepareRootUrls();

    auto newFiles = parseFile();

    if (static_cast<bool>(CConfig::getInstance()["remote_images"]))
        makeRelativeImagesExternal();

    insertAnnoyingAdvertisementThatNobodyWantsToSee();

    save();

    for (const auto &i : newFiles)
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
    ss << "\n\n<!--\n"
       << " This page was mirrored using Wget Clone | https://muj.link/wget \n"
       << " __    __           _     ___ _                  \n"
       << "/ / /\\ \\ \\__ _  ___| |_  / __\\ | ___  _ __   ___ \n"
       << "\\ \\/  \\/ / _` |/ _ \\ __|/ /  | |/ _ \\| '_ \\ / _ \\\n"
       << " \\  /\\  / (_| |  __/ |_/ /___| | (_) | | | |  __/\n"
       << "  \\/  \\/ \\__, |\\___|\\__\\____/|_|\\___/|_| |_|\\___|\n"
       << "         |___/                                   \n"
       << "-->\n";

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

void CFileHtml::makeRelativeImagesExternal()
{
    stringstream replaceString;

    replaceString << "$1\""; // <img src=

    replaceString << m_Url.getNormURL();

    replaceString << "$2\""; // the link itself

    const regex re("(<(?:img) (?:src=))[\"'](?!http:\\/\\/|https:\\/\\/|data:|tel:|javascript:|mailto:|\\/\\/)([^\\/][^\"']*)[\"']", std::regex_constants::icase);
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

    CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "Replaced all '" + searchString + "' with '" + replaceString.str() + "'");
}

set<shared_ptr<CFile>> CFileHtml::parseFile()
{
    set<shared_ptr<CFile>> nextFiles;

    set<string> nextUrls;

    try
    {
        // Match everything in src= and href= that doesn't start with http or https
        const regex re("(?:src=|href=)[\"'](?!http:\\/\\/|https:\\/\\/|data:|tel:|javascript:|mailto:|\\/\\/)([^\"'#]*)(#?[^\"']*)[\"']", std::regex_constants::icase);

        std::sregex_iterator iter(m_Content.begin(), m_Content.end(), re);
        std::sregex_iterator end;

        while (iter != end)
        {
            string value = (*iter)[1];
            iter++;

            if (static_cast<bool>(CConfig::getInstance()["remote_images"]))
            {
                if (Utils::endsWith(value, ".png") ||
                    Utils::endsWith(value, ".jpg") ||
                    Utils::endsWith(value, ".jpeg") ||
                    Utils::endsWith(value, ".webp") ||
                    Utils::endsWith(value, ".gif") ||
                    Utils::endsWith(value, ".png"))
                    continue;
            }

            nextUrls.insert(value);
        }

        if (!static_cast<bool>(CConfig::getInstance()["remote_images"]))
        {
            // Match everything in srcset=
            const regex re2("(?:srcset=)[\"'](?!http:\\/\\/|https:\\/\\/|data:|tel:|javascript:|mailto:|\\/\\/)([^\"'#]*)(#?[^\"']*)[\"']", std::regex_constants::icase);

            std::sregex_iterator iter2(m_Content.begin(), m_Content.end(), re2);
            std::sregex_iterator end2;

            while (iter2 != end2)
            {
                string srcsetContent = (*iter2)[1];

                // Match separately every link in srcset=
                const regex re_srcset("[^\"'=\\s]+\\.[^\\s]+", std::regex_constants::icase);

                std::sregex_iterator iter_srcset(srcsetContent.begin(), srcsetContent.end(), re_srcset);
                std::sregex_iterator end_srcset;

                while (iter_srcset != end_srcset)
                {
                    nextUrls.insert((*iter_srcset)[0]);
                    iter_srcset++;
                }

                iter2++;
            }
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
            newFile = make_shared<CFileHtml>(m_HttpD, m_Depth + 1, newLink);
        else if (Utils::endsWith(newLink.getNormURL(), ".css"))
            newFile = make_shared<CFileCss>(m_HttpD, m_Depth, newLink);
        else
            newFile = make_shared<CFile>(m_HttpD, m_Depth + 1, newLink);

        nextFiles.insert(newFile);
        CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "Next file: " + newLink.getNormURL() + " | (depth " + std::to_string(m_Depth + 1) + ")");
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
            string value = (*iter_external)[1];
            iter_external++;

            if (static_cast<bool>(CConfig::getInstance()["remote_images"]))
            {
                if (Utils::endsWith(value, ".png") ||
                    Utils::endsWith(value, ".jpg") ||
                    Utils::endsWith(value, ".jpeg") ||
                    Utils::endsWith(value, ".webp") ||
                    Utils::endsWith(value, ".gif") ||
                    Utils::endsWith(value, ".png"))
                    continue;
            }

            nextUrls.insert(value);
        }

        if (!static_cast<bool>(CConfig::getInstance()["remote_images"]))
        {
            // Match everything in src= and href= that DOES start with http or https
            const regex re_external_imgs("(?:img) (?:src=|href=)[\"']((?:http:\\/\\/|https:\\/\\/)[^\"'#]*)(#?[^\"']*)[\"']", std::regex_constants::icase);

            std::sregex_iterator iter_external_imgs(m_Content.begin(), m_Content.end(), re_external_imgs);
            std::sregex_iterator end_external_imgs;

            while (iter_external_imgs != end_external_imgs)
            {
                nextUrlsExternal.insert((*iter_external_imgs)[1]);
                iter_external_imgs++;
            }

            // Match everything in srcset=
            const regex re2("(?:srcset=)[\"']((?:http:\\/\\/|https:\\/\\/)[^\"'#]*)(#?[^\"']*)[\"']", std::regex_constants::icase);

            std::sregex_iterator iter2(m_Content.begin(), m_Content.end(), re2);
            std::sregex_iterator end2;

            while (iter2 != end2)
            {
                string srcsetContent = (*iter2)[1];

                // Match separately every link in srcset=
                const regex re_srcset("[^\"'=\\s]+\\.[^\\s]+", std::regex_constants::icase);

                std::sregex_iterator iter_srcset(srcsetContent.begin(), srcsetContent.end(), re_srcset);
                std::sregex_iterator end_srcset;

                while (iter_srcset != end_srcset)
                {
                    nextUrls.insert((*iter_srcset)[0]);
                    iter_srcset++;
                }

                iter2++;
            }
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
