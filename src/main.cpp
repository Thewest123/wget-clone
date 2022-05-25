/**
 * @file main.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <stdlib.h>

#include "CLogger.h"
#include "CConfig.h"
#include "CHttpsDownloader.h"
#include "CFileHtml.h"
#include "CURLHandler.h"

using namespace std;

int main(int argc, char const *argv[])
{

    CLogger::init(CLogger::LogLevel::Info);
    CLogger &logger = CLogger::getInstance();

    CConfig &cfg = CConfig::getInstance();
    if (!cfg.parseArgs(argc, argv))
        return EXIT_FAILURE;

    logger.log(CLogger::LogLevel::Info, "Start");

    // cfg["url"] = "jakpsatweb.cz";
    // cfg["depth"] = 3;
    // cfg["output"] = (string) "./build/jakpsatweb.cz";

    auto httpd = make_shared<CHttpsDownloader>();

    CURLHandler rootUrl(cfg["url"]);

    CFileHtml root(httpd, 1, rootUrl);

    root.download();

    logger.log(CLogger::LogLevel::Info, "End");

    return EXIT_SUCCESS;
}
