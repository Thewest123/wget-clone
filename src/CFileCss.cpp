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

    CLogger::getInstance().log(CLogger::ELogLevel::Verbose, "Processing CSS: " + m_Url.getNormURL() + " | (depth " + std::to_string(m_Depth) + ")");

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

set<shared_ptr<CFile>> CFileCss::parseFile()
{
    set<shared_ptr<CFile>> nextFiles;

    // RELATIVE links
    set<string> nextUrls;

    // Get relative url() links
    auto relUrls = getUrlsWithRegex("(?:url ?\\([\"']?(?!data:|#|http)([^;\"'\\s]+)[\"']?)\\)", m_Content);
    for (const auto &url : relUrls)
    {
        nextUrls.emplace(url);
    }

    // Get relative @import links
    auto relImports = getUrlsWithRegex("(?:@import [\"']?(?!url\\(|data:|#|http)([^;\"'\\s]+)[\"']?)", m_Content);
    for (const auto &import : relImports)
    {
        nextUrls.emplace(import);
    }

    // Transform each URL to correct File and insert into nextFiles set
    transformUrlsToFiles(false, nextUrls, nextFiles);

    // Skip external links if desired
    if (static_cast<bool>(CConfig::getInstance()["remote"]) == true)
        return nextFiles;

    // EXTERNAL links
    set<string> nextUrlsExternal;

    // Get external url() links
    auto externUrls = getUrlsWithRegex("(?:url ?\\([\"']?(?!data:|#)((?:http:\\/\\/|https:\\/\\/)[^;\"'\\s]+)[\"']?)\\)", m_Content);
    for (const auto &url : externUrls)
    {
        nextUrls.emplace(url);
    }

    // Get external @import links
    auto externImports = getUrlsWithRegex("(?:@import [\"']?(?!url\\(|data:|#)((?:http:\\/\\/|https:\\/\\/)[^;\"'\\s]+)[\"']?)", m_Content);
    for (const auto &import : externImports)
    {
        nextUrls.emplace(import);
    }

    // Transform each URL to correct File and insert into nextFiles set
    transformUrlsToFiles(true, nextUrlsExternal, nextFiles);

    return nextFiles;
}
