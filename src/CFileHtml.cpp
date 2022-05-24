/**
 * @file CFile.h
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <stdlib.h>
#include <iostream>
#include <set>
#include <regex>

#include "CFile.h"
#include "CHttpsDownloader.h"
#include "CFileHtml.h"
#include "CLogger.h"

using namespace std;

// CFileHtml::~CFileHtml() = default;

void CFileHtml::download()
{
    CFile::download();

    auto &logger = CLogger::getInstance();
    logger.log(CLogger::LogLevel::Verbose, "HTML verze");

    CConfig &cfg = CConfig::getInstance();

    if ((int)m_Depth >= (int)cfg["max_depth"])
        return;

    auto newFiles = parseFile();

    for (auto &&i : newFiles)
    {
        i->download();
    }
}

set<shared_ptr<CFile>> CFileHtml::parseFile()
{
    set<shared_ptr<CFile>> nextFiles;

    // Match everything in src= and href= that doesn't start with http or https
    const regex re("(?:src=|href=)[\"'](?!http:\\/\\/|https:\\/\\/)([^\"']*)[\"']", regex_constants::icase);

    set<string> nextUrls;

    std::sregex_iterator iter(m_Content.begin(), m_Content.end(), re);
    std::sregex_iterator end;

    while (iter != end)
    {
        nextUrls.insert((*iter)[1]);
        iter++;
    }

    auto &logger = CLogger::getInstance();

    for (auto &&i : nextUrls)
    {
        CURLHandler newLink(m_Url.getNormURL() + i);

        logger.log(CLogger::LogLevel::Verbose, newLink.getNormURL());

        shared_ptr<CFile> newFile;

        if (ends_with(newLink.getNormURL(), ".html"))
            newFile = make_shared<CFileHtml>(m_HttpD, m_Depth + 1, newLink);
        else
            newFile = make_shared<CFile>(m_HttpD, m_Depth + 1, newLink);

        nextFiles.insert(newFile);
    }

    return nextFiles;
}

bool CFileHtml::ends_with(std::string const &value, std::string const &ending)
{
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}