/**
 * @file tests.cpp
 * @author Jan Cerny (cernyj87@fit.cvut.cz)
 */

#ifdef IS_TESTS

#include "Utils.h"
#include "CURLHandler.h"
#include "CConfig.h"
#include "CLogger.h"

#include <algorithm>

using std::string, std::cout, std::endl, std::boolalpha;

#define ASSERT(x)                                                                                                   \
     {                                                                                                              \
          if (!(x))                                                                                                 \
          {                                                                                                         \
               std::cout << "\033[31m[FAIL]\033[0m Line " << __LINE__ << " : " << __PRETTY_FUNCTION__ << std::endl; \
               Tests::ALL_PASSED = false;                                                                           \
          }                                                                                                         \
          else                                                                                                      \
               std::cout << "\033[32m[PASS]\033[0m Line " << __LINE__ << " : " << __PRETTY_FUNCTION__ << std::endl; \
     }

namespace Tests
{
     static bool ALL_PASSED;

     void Utils_startsWith()
     {
          string str = "../test/loremipsum.html";

          ASSERT(Utils::startsWith(str, "../"));
          ASSERT(Utils::startsWith(str, "."));
          ASSERT(Utils::startsWith(str, str));
          ASSERT(!Utils::startsWith(str, "x"));
          ASSERT(!Utils::startsWith(str, "./test"));
          ASSERT(!Utils::startsWith(str, " "));
          ASSERT(Utils::startsWith(str, ""));
     }

     void Utils_endsWith()
     {
          string str = "../test/loremipsum.html";

          ASSERT(Utils::endsWith(str, "l"));
          ASSERT(Utils::endsWith(str, ".html"));
          ASSERT(Utils::endsWith(str, str));
          ASSERT(!Utils::endsWith(str, "x"));
          ASSERT(!Utils::endsWith(str, ".htm"));
          ASSERT(!Utils::endsWith(str, " "));
          ASSERT(Utils::endsWith(str, ""));
     }

     void Utils_contains()
     {
          string str = "../test/loremipsum.html";

          ASSERT(Utils::contains(str, "l"));
          ASSERT(Utils::contains(str, "loremipsum"));
          ASSERT(Utils::contains(str, ".html"));
          ASSERT(Utils::contains(str, "../"));
          ASSERT(!Utils::contains(str, "..."));
          ASSERT(!Utils::contains(str, "\n"));
          ASSERT(!Utils::contains(str, " "));
          ASSERT(Utils::contains(str, ""));
     }

     void Utils_toLowerCase()
     {
          ASSERT(Utils::toLowerCase("lorem") == "lorem");
          ASSERT(Utils::toLowerCase("LOREM") == "lorem");
          ASSERT(Utils::toLowerCase("lOrEm") == "lorem");
          ASSERT(Utils::toLowerCase("  lorem IPSum") == "  lorem ipsum");
          ASSERT(Utils::toLowerCase("+ěščřžýáíé") == "+ěščřžýáíé");
          ASSERT(Utils::toLowerCase("0123456789") == "0123456789");
          ASSERT(Utils::toLowerCase("ř - LOREM Ipsum! 123\n") == "ř - lorem ipsum! 123\n");
     }

     void Utils_replaceAll()
     {
          string replace = "lorem sit ipusm dolor sit amet 123";

          ASSERT(Utils::replaceAll(replace, "i", "y") == 3);
          ASSERT(replace == "lorem syt ypusm dolor syt amet 123");

          ASSERT(Utils::replaceAll(replace, "syt", "longerText") == 2);
          ASSERT(replace == "lorem longerText ypusm dolor longerText amet 123");

          try
          {
               Utils::replaceAll(replace, "", "neco");
               ASSERT("Missing exception!" == nullptr)
          }
          catch (const std::invalid_argument &e)
          {
               ASSERT(string(e.what()) == "'what' argument cannot be empty!");
          }

          ASSERT(Utils::replaceAll(replace, "dolor", "") == 1);
          ASSERT(replace == "lorem longerText ypusm  longerText amet 123");

          try
          {
               Utils::replaceAll(replace, replace, "");
               ASSERT("Missing exception!" == nullptr)
          }
          catch (const std::invalid_argument &e)
          {
               ASSERT(string(e.what()) == "'what' argument cannot be equal to 'str'!");
          }
     }

     void Utils_splitString()
     {
          string input = "lorem/ipsum/./dolor/sit//amet///";
          string delimiter = "/";

          vector<string> output = Utils::splitString(input, delimiter);

          ASSERT(output.size() == 10);
          ASSERT(output[0] == "lorem");
          ASSERT(output[1] == "ipsum");
          ASSERT(output[2] == ".");
          ASSERT(output[3] == "dolor");
          ASSERT(output[4] == "sit");
          ASSERT(output[5] == "");
          ASSERT(output[6] == "amet");
          ASSERT(output[7] == "");
          ASSERT(output[8] == "");
          ASSERT(output[9] == "");
     }

     void CURLHandler_construct()
     {
          CURLHandler url("https://www.google.com/index.html/");

          ASSERT(url.isHttps());
          ASSERT(!url.isExternal());
          ASSERT(url.getDomain() == "www.google.com");
          ASSERT(url.getDomainNorm() == "google.com");
          ASSERT(url.getNormURL() == "https://www.google.com/index.html");
          ASSERT(url.getNormURLPath() == "index.html");
          ASSERT(url.getNormFilePath() == "index.html");
          ASSERT(url.getPathDepth() == 0);
     }

     void CURLHandler_setDomain()
     {
          CURLHandler url("https://www.google.com/index.html/");
          url.setDomain("fit.cvut.cz");

          ASSERT(url.isHttps());
          ASSERT(!url.isExternal());
          ASSERT(url.getDomain() == "fit.cvut.cz");
          ASSERT(url.getDomainNorm() == "fit.cvut.cz");
          ASSERT(url.getNormURL() == "https://fit.cvut.cz/index.html");
          ASSERT(url.getNormURLPath() == "index.html");
          ASSERT(url.getNormFilePath() == "index.html");
          ASSERT(url.getPathDepth() == 0);
     }

     void CURLHandler_addPath()
     {
          CURLHandler url("https://www.google.com");

          url.addPath("lorem//ipsum/./dolor/../../folder");

          ASSERT(url.getNormURL() == "https://www.google.com/lorem/folder/");
          ASSERT(url.getNormURLPath() == "lorem/folder");
          ASSERT(url.getNormFilePath() == "lorem/folder/");
          ASSERT(url.getPathDepth() == 2);

          url.addPath("next/file.js");
          ASSERT(url.getNormURLPath() == "lorem/folder/next/file.js");
          ASSERT(url.getNormFilePath() == "lorem/folder/next/file.js");
          ASSERT(url.getPathDepth() == 3);
     }

     void CConfig_storeValues()
     {
          CConfig &cfg = CConfig::getInstance();

          ASSERT(static_cast<string>(cfg["empty"]) == string(""));

          cfg["lorem"] = string("ipsum");
          cfg["num"] = 123;
          cfg["boolean"] = true;
     }

     void CConfig_getValues()
     {
          CConfig &cfg = CConfig::getInstance();

          ASSERT(static_cast<string>(cfg["empty"]) == string(""));

          try
          {
               static_cast<int>(cfg["empty"]);
               ASSERT("Missing exception!" == nullptr)
          }
          catch (const std::invalid_argument &e)
          {
               ASSERT(string(e.what()) == "stoi");
          }

          ASSERT(static_cast<bool>(cfg["empty"]) == false);

          ASSERT(static_cast<string>(cfg["lorem"]) == string("ipsum"));

          ASSERT(static_cast<int>(cfg["num"]) == 123);

          ASSERT(static_cast<bool>(cfg["boolean"]) == true);
     }

     void CConfig_parseArgsMissingUrl()
     {
          int argCount = 2;
          const char *argVal[2] = {
              "./wget.out",
              "-q"};

          CConfig &cfg = CConfig::getInstance();
          ASSERT(cfg.parseArgs(argCount, argVal) == false);
     }

     void CConfig_parseArgsInvalid()
     {
          int argCount = 7;
          const char *argVal[7] = {
              "./wget.out",
              "--unknown",
              "2",
              "google.com",
              "-q",
              "--output",
              "./folder"};

          CConfig &cfg = CConfig::getInstance();
          ASSERT(cfg.parseArgs(argCount, argVal) == false);
     }

     void CConfig_parseArgsValid()
     {
          int argCount = 7;
          const char *argVal[7] = {
              "./wget.out",
              "-d",
              "2",
              "google.com",
              "-q",
              "--output",
              "./folder"};

          CConfig &cfg = CConfig::getInstance();
          ASSERT(cfg.parseArgs(argCount, argVal) == true);
          ASSERT(static_cast<int>(cfg["depth"]) == 2);
          ASSERT(static_cast<string>(cfg["url"]) == "google.com");
          ASSERT(static_cast<int>(cfg["log_level"]) == 2);
          ASSERT(static_cast<string>(cfg["output"]) == "./folder");
     }

} // namespace Tests

int main(void)
{
     Tests::ALL_PASSED = true;

     cout << "---------- [STARTING TESTS] ----------\n"
          << endl;

     // ============ Utils ============
     cout << "---------- [Testing Utils] -----------" << endl;

     Tests::Utils_startsWith();
     Tests::Utils_endsWith();
     Tests::Utils_contains();
     Tests::Utils_toLowerCase();
     Tests::Utils_replaceAll();
     Tests::Utils_splitString();

     cout << endl;

     // ============ CURLHandler ============
     cout << "------- [Testing CURLHandler] --------" << endl;

     Tests::CURLHandler_construct();
     Tests::CURLHandler_addPath();
     Tests::CURLHandler_setDomain();

     cout << endl;

     // ============ CConfig ============
     cout << "------- [Testing CConfig] --------" << endl;

     CLogger::init(CLogger::ELogLevel::Error);
     Tests::CConfig_storeValues();
     Tests::CConfig_getValues();
     Tests::CConfig_parseArgsMissingUrl();
     Tests::CConfig_parseArgsInvalid();
     Tests::CConfig_parseArgsValid();

     cout << endl;

     // ============ END ============
     if (Tests::ALL_PASSED)
          cout << "\n--------- \033[32m[ALL TESTS PASSED]\033[0m ---------\n"
               << endl;
     else
          cout << "\n--------- \033[31m[SOME TESTS FAILED]\033[0m --------\n"
               << endl;

     return EXIT_SUCCESS;
}

#endif