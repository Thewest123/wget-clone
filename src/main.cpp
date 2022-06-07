/**
 * @file main.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#ifndef IS_TESTS

#include "CLogger.h"
#include "CConfig.h"
#include "CHttpsDownloader.h"
#include "CFileHtml.h"
#include "CURLHandler.h"

#include <stdlib.h>

// using namespace std;
using std::string, std::make_shared;

int main(int argc, char const *argv[])
{
    // Init Logger
    CLogger::init(CLogger::LogLevel::Info);
    CLogger &logger = CLogger::getInstance();

    // Init Config and parse input
    CConfig &cfg = CConfig::getInstance();
    if (!cfg.parseArgs(argc, argv))
        return EXIT_FAILURE;

    // Create Https downloader
    auto httpd = make_shared<CHttpsDownloader>();

    // Create Root URL
    CURLHandler rootUrl((string)cfg["url"]);

    // Create root HTML file
    CFileHtml root(httpd, 1, rootUrl);

    // Download the file and recursively other linked files
    try
    {
        root.download();
    }
    catch (std::exception &e)
    {
        CLogger::getInstance().log(CLogger::LogLevel::Error, e.what());
        return EXIT_FAILURE;
    }

    // Exit
    logger.log(CLogger::LogLevel::Info, "Done.");
    return EXIT_SUCCESS;
}
#endif