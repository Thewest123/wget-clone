/**
 * @file main.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <stdlib.h>

#include "CLogger.h"
#include "CConfig.h"
#include "CHttpsDownloader.h"
#include "CFileHtml.h"

using namespace std;

int main(int argc, char const *argv[])
{

    CLogger::init(CLogger::LogLevel::Verbose);

    CLogger &logger = CLogger::getInstance();
    logger.log(CLogger::LogLevel::Info, "Start");
    
    CConfig &cfg = CConfig::getInstance();    
    cfg["max_depth"] = 2;

    auto httpd = make_shared<CHttpsDownloader>();

    string rootUrl = "https://www.jakpsatweb.cz/";

    CFileHtml root(httpd, 1, rootUrl);

    root.download();

    logger.log(CLogger::LogLevel::Info, "End");

    return EXIT_SUCCESS;
}
