/**
 * @file main.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#include <stdlib.h>

#include "CLogger.h"
#include "CConfig.h"
#include "CHttpDownloader.h"
#include "CFileHtml.h"

using namespace std;

int main(int argc, char const *argv[])
{

    CLogger::init(CLogger::LogLevel::Verbose);

    CLogger &logger = CLogger::getInstance();
    logger.log(CLogger::LogLevel::Verbose, "Start");
    
    CConfig &cfg = CConfig::getInstance();    
    cfg["max_depth"] = 10;

    auto httpd = make_shared<CHttpDownloader>();

    string rootUrl = "http://www.google.com/";

    CFileHtml root(httpd, 1, rootUrl);

    root.download();

    return EXIT_SUCCESS;
}
