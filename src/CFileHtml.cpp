/**
 * @file CFileHtml.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Implementation of CFileHtml
 *
 */

#include "CFileHtml.h"
#include "CFileCss.h"
#include "CLogger.h"
#include "CConfig.h"
#include "Utils.h"

#include <stdlib.h>
#include <iostream>
#include <set>
#include <regex>
#include <filesystem>
#include <fstream>

#include <memory> // shared_ptr<>
#include <string>

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

    CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "Processing HTML: " + m_Url.getNormURL() + " | (depth " + std::to_string(m_Depth) + ")");

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

set<shared_ptr<CFile>> CFileHtml::parseFile()
{
    set<shared_ptr<CFile>> nextFiles;

    // RELATIVE links
    set<string> nextUrls;

    // Get relative src= and href=
    auto relSrcHrefs = getUrlsWithRegex("(?:src=|href=)[\"'](?!http:\\/\\/|https:\\/\\/|data:|tel:|javascript:|mailto:|\\/\\/)([^\"'#]*)(#?[^\"']*)[\"']", m_Content);
    for (const auto &srcHref : relSrcHrefs)
    {
        // If remote images, skip them
        if (static_cast<bool>(CConfig::getInstance()["remote_images"]))
        {
            if (Utils::endsWith(srcHref, ".png") ||
                Utils::endsWith(srcHref, ".jpg") ||
                Utils::endsWith(srcHref, ".jpeg") ||
                Utils::endsWith(srcHref, ".webp") ||
                Utils::endsWith(srcHref, ".gif") ||
                Utils::endsWith(srcHref, ".png"))
                continue;
        }

        nextUrls.emplace(srcHref);
    }

    // If NOT remote images, also get relative srcset=
    if (!static_cast<bool>(CConfig::getInstance()["remote_images"]))
    {
        auto relSourcesets = getUrlsWithRegex("(?:srcset=)[\"'](?!http:\\/\\/|https:\\/\\/|data:|tel:|javascript:|mailto:|\\/\\/)([^\"'#]*)(#?[^\"']*)[\"']", m_Content);

        // Get each sourceset tag
        for (const auto &sourceset : relSourcesets)
        {
            // Get each URL in one sourceset tag
            auto sourcesetUrls = getUrlsWithRegex("([^\"'=\\s]+\\.[^\\s]+)", sourceset);
            for (const auto &url : sourcesetUrls)
            {
                if (!Utils::startsWith(url, "/"))
                    nextUrls.emplace(url);
            }
        }
    }

    // Transform each URL to correct File and insert into nextFiles set
    transformUrlsToFiles(false, nextUrls, nextFiles);

    // Skip external links if desired
    if (static_cast<bool>(CConfig::getInstance()["remote"]) == true)
        return nextFiles;

    // EXTERNAL links
    set<string> nextUrlsExternal;

    // Get external src= and href=
    auto externSrcHrefs = getUrlsWithRegex("(?:src=|href=)[\"']((?:http:\\/\\/|https:\\/\\/)[^\"'#]*)(#?[^\"']*)[\"']", m_Content);
    for (const auto &srcHref : externSrcHrefs)
    {
        // If remote images, skip them
        if (static_cast<bool>(CConfig::getInstance()["remote_images"]))
        {
            if (Utils::endsWith(srcHref, ".png") ||
                Utils::endsWith(srcHref, ".jpg") ||
                Utils::endsWith(srcHref, ".jpeg") ||
                Utils::endsWith(srcHref, ".webp") ||
                Utils::endsWith(srcHref, ".gif") ||
                Utils::endsWith(srcHref, ".png"))
                continue;
        }

        nextUrlsExternal.emplace(srcHref);
    }

    // Get external srcset=
    if (!static_cast<bool>(CConfig::getInstance()["remote_images"]))
    {
        auto relSourcesets = getUrlsWithRegex("(?:srcset=)[\"'](?!http:\\/\\/|https:\\/\\/|data:|tel:|javascript:|mailto:|\\/\\/)([^\"'#]*)(#?[^\"']*)[\"']", m_Content);

        // Get each sourceset tag
        for (const auto &sourceset : relSourcesets)
        {
            // Get each URL in one sourceset tag
            auto sourcesetUrls = getUrlsWithRegex("([^\"'=\\s]+\\.[^\\s]+)", sourceset);
            for (const auto &url : sourcesetUrls)
            {
                if (!Utils::startsWith(url, "/"))
                    nextUrlsExternal.emplace(url);
            }
        }
    }

    // Transform each URL to correct File and insert into nextFiles set
    transformUrlsToFiles(true, nextUrlsExternal, nextFiles);

    return nextFiles;
}
