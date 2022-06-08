/**
 * @file main.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 * @brief Entry point for Wget Clone
 */

#ifndef IS_TESTS

#include "CLogger.h"
#include "CConfig.h"
#include "CHttpsDownloader.h"
#include "CFile.h"
#include "CFileHtml.h"
#include "CFileCss.h"
#include "CURLHandler.h"
#include "Utils.h"

#include <stdlib.h>

// using namespace std;
using std::string, std::make_shared;

int main(int argc, char const *argv[])
{
    // Init Logger
    CLogger::init(CLogger::ELogLevel::Info);
    CLogger &logger = CLogger::getInstance();

    // Init Config and parse input
    CConfig &cfg = CConfig::getInstance();
    if (!cfg.parseArgs(argc, argv))
        return EXIT_SUCCESS;

    // Create Https downloader
    auto httpd = make_shared<CHttpsDownloader>();

    // Create Root URL
    CURLHandler rootUrl((string)cfg["url"]);

    // Create root HTML file
    shared_ptr<CFile> rootFile;

    if (Utils::endsWith(rootUrl.getNormURL(), ".html") || Utils::endsWith(rootUrl.getNormURL(), ".php") || Utils::endsWith(rootUrl.getNormURL(), "/"))
        rootFile = make_shared<CFileHtml>(httpd, 1, rootUrl);
    else if (Utils::endsWith(rootUrl.getNormURL(), ".css"))
        rootFile = make_shared<CFileCss>(httpd, 1, rootUrl);
    else
        rootFile = make_shared<CFile>(httpd, 1, rootUrl);

    // Download the file and recursively other linked files
    try
    {
        rootFile->download();
    }
    catch (std::exception &e)
    {
        CLogger::getInstance().log(CLogger::ELogLevel::Error, e.what());
        return EXIT_FAILURE;
    }

    // Exit
    logger.log(CLogger::ELogLevel::Info, "Done.");
    return EXIT_SUCCESS;
}
#endif